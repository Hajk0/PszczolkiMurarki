#include "main.h"
#include "watek_komunikacyjny.h"
#include <limits.h>

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;//
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&clock_mutex);
        if (pakiet.ts > lamport_clock) {
            lamport_clock = pakiet.ts;
        }
        lamport_clock++;
        pthread_mutex_unlock(&clock_mutex);

        switch ( status.MPI_TAG ) {
	    case REQUEST: 
            pthread_mutex_lock(&check_cond_mutex);
            n_req_ts[pakiet.data][pakiet.src] = pakiet.ts;
            if (onNTopQueue(rank, 1, pakiet.data)) { //CRIT_SEC_SIZE
                pthread_cond_signal(&check_cond);
            }
            pthread_mutex_unlock(&check_cond_mutex);
                debug("Ktoś coś prosi. A niech ma!")
		    sendPacket( 0, status.MPI_SOURCE, ACK );
	        break;
	    case ACK: 
            pthread_mutex_lock(&check_cond_mutex);
	        ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
            if (ackCount >= size - 1) { //
                pthread_cond_signal(&check_cond);
            }
            pthread_mutex_unlock(&check_cond_mutex);
                debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
	        break;
        case RELEASE:
                debug("Otrzymałem wiadomość, że proces %d zwalnia sekcję krytyczną", status.MPI_SOURCE);
            n_req_ts[pakiet.data][pakiet.src] = INT_MAX;
            pthread_mutex_lock(&check_cond_mutex);
            if (onNTopQueue(rank, 1, pakiet.data)) { //CRIT_SEC_SIZE
                pthread_cond_signal(&check_cond);
            }
            pthread_mutex_unlock(&check_cond_mutex);
            break;
        case FLOWER_REQUEST:
            pthread_mutex_lock(&check_cond_flower_mutex);
            flower_req_ts[pakiet.src] = pakiet.ts;
            if (onFlowerTopQueue(rank, CRIT_SEC_SIZE, pakiet.data)) {
                pthread_cond_signal(&check_cond_flower);
            }
            pthread_mutex_unlock(&check_cond_flower_mutex);
            debug("Ktoś coś prosi. A niech ma!");
            sendPacket( 0, status.MPI_SOURCE, FLOWER_ACK );
            break;
        case FLOWER_RELEASE:
            debug("Otrzymałem wiadomość, że proces %d zwalnia sekcję krytyczną", status.MPI_SOURCE);
            flower_req_ts[pakiet.src] = INT_MAX;
            pthread_mutex_lock(&check_cond_flower_mutex);
            if (onFlowerTopQueue(rank, CRIT_SEC_SIZE, pakiet.data)) {
                pthread_cond_signal(&check_cond_flower);
            }
            pthread_mutex_unlock(&check_cond_flower_mutex);
            break;
        case FLOWER_ACK:
            pthread_mutex_lock(&check_cond_flower_mutex);
            flower_ackCount++;
            if (flower_ackCount >= size - CRIT_SEC_SIZE) { //
                pthread_cond_signal(&check_cond_flower);
            }
            pthread_mutex_unlock(&check_cond_flower_mutex);
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, flower_ackCount);
            break;
        case APP_PKT:
            debug("Otrzymałem wiadomość od %d", status.MPI_SOURCE);
            reed_capacity[pakiet.data]--;
            println("Otrzymałem kwiatek od %d", status.MPI_SOURCE);
            break;
	    default:
	        break;
        }
    }
}
