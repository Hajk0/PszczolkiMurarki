#include "main.h"
#include "watek_komunikacyjny.h"
#include <limits.h>

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
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
        timestamps[pakiet.src] = pakiet.ts;
        pthread_mutex_unlock(&clock_mutex);

        switch ( status.MPI_TAG ) {
	    case REQUEST: 
            req_ts[pakiet.src] = pakiet.ts;
                debug("Ktoś coś prosi. A niech ma!")
		    sendPacket( 0, status.MPI_SOURCE, ACK );
	        break;
	    case ACK: 
	        ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
                debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
	        break;
        case RELEASE:
                debug("Otrzymałem wiadomość, że proces %d zwalnia sekcję krytyczną", status.MPI_SOURCE);
            req_ts[pakiet.src] = INT_MAX;
            break;
	    default:
	        break;
        }
    }
}
