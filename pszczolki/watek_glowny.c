#include "main.h"
#include "watek_glowny.h"
#include <limits.h>

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;
	int hp = 5;

    while (stan != InFinish) {
		switch (stan) {
			case InRun: 
			perc = random()%10;
			
			if ( perc < REED_AMOUNT && reed_capacity[perc] > 0) {
				debug("Perc: %d", perc);
				println("Ubiegam się o sekcję krytyczną")
				debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				sendRequests(pkt);
		
				changeState( InWantReet ); 

				free(pkt);
			}
			debug("Skończyłem myśleć");
			break;
			case InWantReet:
			
			for (int i = 0; i < REED_AMOUNT; i++) {
				printf("%d ", reed_capacity[i]);
			}
			printf("\n");
			if (reed_capacity[perc] == 0) {
				changeState( InRun );
				break;
			}

			println("Czekam na wejście do sekcji krytycznej")
			pthread_mutex_lock(&check_cond_mutex);
			if ( ackCount >= size - 1 && onNTopQueue(rank, 1, perc)) { //CRIT_SEC_SIZE
				for (int i = 0; i < REED_AMOUNT; i++) {
					for (int j = 0; j < PROC_AMOUNT; j++) {
						printf("%d ", n_req_ts[i][j]);
					}
					printf("\n");
				}
				
				pthread_mutex_unlock(&check_cond_mutex);
				changeState(InSectionReet);
				break;
			} else {
				pthread_cond_wait(&check_cond, &check_cond_mutex);
				pthread_mutex_unlock(&check_cond_mutex);
			}
			break;
			case InSectionReet:
			// tutaj zapewne jakiś muteks albo zmienna warunkowa

				println("Jestem w sekcji krytycznej (na trzcinie)")
				printf("Perc: %d\n", perc);
				sleep(5);
			//if ( perc < 25 ) {
				// TODO(obsługa kwiatów)
				changeState( InRunFlower );
				
			//}
			break;
			case InRunFlower:
				println("Ubiegam się o sekcję krytyczną (kwiatek)")
				debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				sendRequestsFlower(pkt);
				for (int i = 0; i < PROC_AMOUNT; i++) {
					printf("%d ", flower_req_ts[i]);
				}
		
				changeState( InWantFlower ); 

				free(pkt);
				debug("Skończyłem myśleć");
				break;
			case InWantFlower:
				for (int i = 0; i < PROC_AMOUNT; i++) { ///
					printf("%d ", flower_req_ts[i]);
				}
				println("Czekam na wejście do sekcji krytycznej (na kwiatek)");
				
				pthread_mutex_lock(&check_cond_flower_mutex);
				if ( flower_ackCount >= size - CRIT_SEC_SIZE && onFlowerTopQueue(rank, CRIT_SEC_SIZE, perc)) { //
					for (int j = 0; j < PROC_AMOUNT; j++) {
						printf("%d ", flower_req_ts[j]);
					}
					
					pthread_mutex_unlock(&check_cond_flower_mutex);
					changeState(InSectionFlower);
					break;
				} else {
					pthread_cond_wait(&check_cond_flower, &check_cond_flower_mutex);
					pthread_mutex_unlock(&check_cond_flower_mutex);
				}
				break;
			case InSectionFlower:
				println("Jestem w sekcji krytycznej (na kwiatku)")
				printf("Perc: %d\n", perc);
				sleep(5);
				if ( hp > 0 ) {
					changeState( InRunFlower );
					hp--;
					packet_t *pkt = malloc(sizeof(packet_t));
					pkt->data = perc;
					for (int i = 0; i < size; i++) {
						if (i != rank) {
							sendPacket( pkt, i, APP_PKT );
						} else {
							reed_capacity[perc]--;
							pthread_mutex_lock(&clock_mutex);
							lamport_clock++;
							pthread_mutex_unlock(&clock_mutex);
						}
					}
				} else {
					changeState( Agony );
				}

				debug("Perc: %d", perc);
				println("Wychodzę z sekcji krytycznej (z kwiatka)")
				debug("Zmieniam stan na wysyłanie");
				// packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				sendReleasesFlower(pkt);
				free(pkt);
				break;
			case Agony:
				debug("Perc: %d", perc);
				println("Wychodzę z sekcji krytycznej (z trzciny)")
				debug("Zmieniam stan na wysyłanie");
				// packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				sendReleases(pkt);
				changeState( Dead );
				free(pkt);
			case Dead:
				println("Zakończyłem pracę")
				sleep(5);
				break;
			default: 
			break;
		}
        sleep(SEC_IN_STATE);
    }
}
