#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
#define STATE_CHANGE_PROB 10
#define PROC_AMOUNT 8
#define CRIT_SEC_SIZE 2
#define REED_AMOUNT 2
// #define DEBUG 1

#define ROOT 0

/* tutaj TYLKO zapowiedzi - definicje w main.c */
extern int rank;
extern int size;
extern int ackCount;
extern int flower_ackCount;
extern pthread_t threadKom;

extern int lamport_clock;
extern pthread_mutex_t clock_mutex;
extern pthread_mutex_t check_cond_mutex;
extern pthread_cond_t check_cond;
extern pthread_mutex_t check_cond_flower_mutex;
extern pthread_cond_t check_cond_flower;

extern int timestamps[PROC_AMOUNT];
extern int req_ts[PROC_AMOUNT];
extern int n_req_ts[REED_AMOUNT][PROC_AMOUNT];
extern int flower_req_ts[PROC_AMOUNT];
extern int reed_capacity[REED_AMOUNT];

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d] [clock: %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamport_clock, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT,...) printf("%c[%d;%dm [%d] [clock: %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamport_clock, ##__VA_ARGS__, 27,0,37);


#endif
