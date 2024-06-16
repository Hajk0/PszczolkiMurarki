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
		perc = random()%100;
		if ( perc < 25 ) {
		    debug("Perc: %d", perc);
		    println("Ubiegam się o sekcję krytyczną")
		    debug("Zmieniam stan na wysyłanie");
		    packet_t *pkt = malloc(sizeof(packet_t));
		    pkt->data = perc;
			sendRequests(pkt);
			
		    changeState( InWant ); // w VI naciśnij ctrl-] na nazwie funkcji, ctrl+^ żeby wrócić
					   // :w żeby zapisać, jeżeli narzeka że w pliku są zmiany
					   // ewentualnie wciśnij ctrl+w ] (trzymasz ctrl i potem najpierw w, potem ]
					   // między okienkami skaczesz ctrl+w i strzałki, albo ctrl+ww
					   // okienko zamyka się :q
					   // ZOB. regułę tags: w Makefile (naciśnij gf gdy kursor jest na nazwie pliku)
		    free(pkt);
		} // a skoro już jesteśmy przy komendach vi, najedź kursorem na } i wciśnij %  (niestety głupieje przy komentarzach :( )
		debug("Skończyłem myśleć");
		break;
	    case InWant:
		println("Czekam na wejście do sekcji krytycznej")
		// tutaj zapewne jakiś semafor albo zmienna warunkowa
		// bo aktywne czekanie jest BUE
		pthread_mutex_lock(&check_cond_mutex);
		if ( ackCount >= size - CRIT_SEC_SIZE && onNTopQueue(rank, CRIT_SEC_SIZE)) { //
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
