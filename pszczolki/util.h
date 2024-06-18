#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  

    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK     1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH  5
#define FLOWER_REQUEST 6
#define FLOWER_RELEASE 7
#define FLOWER_ACK 8

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum {InRun, InMonitor, InWantReet, InSectionReet, InRunFlower, InWantFlower, InSectionFlower, Agony, InFinish, Dead} state_t;
extern state_t stan;
extern pthread_mutex_t stateMut;
/* zmiana stanu, obwarowana muteksem */
void changeState( state_t );
int onTopQueue(int rank, int perc);
int onNTopQueue(int rank, int topN, int perc);
void sendRequests(packet_t *pkt);
void sendReleases(packet_t *pkt);
int onFlowerTopQueue(int rank, int topN, int perc);
void sendRequestsFlower(packet_t *pkt);
void sendReleasesFlower(packet_t *pkt);
#endif
