#include "main.h"
#include "watek_glowny.h"
#include <limits.h>

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    while (stan != InFinish) {
		switch (stan) {
			case InRun: 
			perc = random()%10;
			if ( perc < REET_AMOUNT ) {
				debug("Perc: %d", perc);
				println("Ubiegam się o sekcję krytyczną")
				debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				sendRequests(pkt);
		
				changeState( InWant ); 

				free(pkt);
			}
			debug("Skończyłem myśleć");
			break;
			case InWant:
			println("Czekam na wejście do sekcji krytycznej")
			// tutaj zapewne jakiś semafor albo zmienna warunkowa
			// bo aktywne czekanie jest BUE
			pthread_mutex_lock(&check_cond_mutex);
			if ( ackCount >= size - CRIT_SEC_SIZE && onNTopQueue(rank, CRIT_SEC_SIZE, perc)) { //
				for (int i = 0; i < REET_AMOUNT; i++) {
					for (int j = 0; j < PROC_AMOUNT; j++) {
						printf("%d ", n_req_ts[i][j]);
					}
					printf("\n");
				}
				
				pthread_mutex_unlock(&check_cond_mutex);
				changeState(InSection);
				break;
			} else {
				pthread_cond_wait(&check_cond, &check_cond_mutex);
				pthread_mutex_unlock(&check_cond_mutex);
			}
			break;
			case InSection:
			// tutaj zapewne jakiś muteks albo zmienna warunkowa

				println("Jestem w sekcji krytycznej")
				printf("Perc: %d\n", perc);
				sleep(5);
			//if ( perc < 25 ) {
				debug("Perc: %d", perc);
				println("Wychodzę z sekcji krytycznej")
				debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				sendReleases(pkt);
				changeState( InRun );
				free(pkt);
			//}
			break;
			default: 
			break;
		}
        sleep(SEC_IN_STATE);
    }
}
