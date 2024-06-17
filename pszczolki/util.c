#include "main.h"
#include "util.h"
#include <limits.h>
MPI_Datatype MPI_PAKIET_T;

/* 
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
state_t stan=InRun;

/* zamek wokół zmiennej współdzielonej między wątkami. 
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;

struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = { { "pakiet aplikacyjny", APP_PKT }, { "finish", FINISH}, 
                { "potwierdzenie", ACK}, {"prośbę o sekcję krytyczną", REQUEST}, {"zwolnienie sekcji krytycznej", RELEASE} };

const char *const tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}

void inicjuj_typ_pakietu()
{
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;

    pthread_mutex_lock(&clock_mutex);
    lamport_clock++;
    pkt->ts = lamport_clock;
    pthread_mutex_unlock(&clock_mutex);

    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string( tag), destination);
    if (freepkt) free(pkt);
}

void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    if (stan==InFinish) { 
	pthread_mutex_unlock( &stateMut );
        return;
    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}

int onTopQueue(int rank, int perc) {
    for (int i = 0; i < PROC_AMOUNT; i++) {
        if (i != rank && n_req_ts[perc][i] < n_req_ts[perc][rank]) {
            return FALSE;
        }
    }
    return TRUE;
}

int onNTopQueue(int rank, int topN, int perc) {
    int count = 0;
    for (int i = 0; i < PROC_AMOUNT; i++) {
        if (i != rank && n_req_ts[perc][i] <= n_req_ts[perc][rank]) {
            count++;
            if (count == topN) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void sendRequests(packet_t *pkt) {
    ackCount = 0;

    for (int i=0;i<=size-1;i++) {
        if (i!=rank) {
            sendPacket( pkt, i, REQUEST);
        } else { // send to myself
            pthread_mutex_lock(&clock_mutex);
            lamport_clock++;
            n_req_ts[pkt->data][rank] = lamport_clock;
            pthread_mutex_unlock(&clock_mutex);
        }
    }
}

void sendReleases(packet_t *pkt) {
    for (int i=0;i<=size-1;i++) {
        if (i!=rank) {
            sendPacket( pkt, i, RELEASE);
        } else { // send to myself
            pthread_mutex_lock(&clock_mutex);
            lamport_clock++;
            n_req_ts[pkt->data][rank] = INT_MAX;
            pthread_mutex_unlock(&clock_mutex);
        }
    }
}