#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include "specifications.h"
#include <sys/msg.h>
#include <time.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int sharedmem = -1;
int semaphore = -1;
Shm *shm;

long get_time(){
    long timer;
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    timer = t.tv_nsec / 1000;
    return timer;
}

void sighandler (int signo)
{
    exit(0);
}

void removeshmandsem ()
{
    if(shmctl(sharedmem, IPC_RMID, NULL) < 0) printf("GOLIBRODA: Something went wrong while deleting shm.\n");
    if(semctl(semaphore, 0, IPC_RMID, NULL) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
}

int main(int argc, char *argv[]) {
    if (atexit(removeshmandsem) < 0) FAILURE_EXIT(1, "GOLIBRODA: Couldn't register atexit function.\n");
    if (argc != 2 ) FAILURE_EXIT(1, "GOLIBRODA: Pass number of seats.\n");
    int seatlimit = (int) strtol(argv[1], NULL, 10);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    key_t semakey = ftok(semaphorepath, PROJ_ID);
    if ((semaphore= semget(semakey, 7, IPC_CREAT | 0666)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");
    key_t shmkey = ftok(shmpath, PROJ_ID);
    if ((sharedmem = shmget(shmkey, sizeof(Shm), IPC_CREAT | 0666)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make shmem.\n");
    if ((shm = (Shm *) shmat(sharedmem, NULL, 0)) < 0) FAILURE_EXIT(1, "GOLIBRODA: Couldn't get pointer to shm.\n");
    shm->seatlimit = seatlimit;
    shm->clientscounter = 0;
    for (int j = 0; j <2; ++j) { // odblokowanie semaforow golibrody
        semctl(semaphore, j, SETVAL, 1);
    }
    for (int j = 2; j <7; ++j) { //zablokowanie semaforow odpowiedzialnych za strzyzenie
        semctl(semaphore, j, SETVAL, 0);
    }
    for (int i = 0; i <seatlimit ; ++i) {
        shm->seats[i] = 0;
    }
    struct sembuf waitsem; // semafor do czekania na pozwolenie
    waitsem.sem_op = -1;
    waitsem.sem_flg = 0;
    struct sembuf unblocksem;
    unblocksem.sem_op = 1;
    unblocksem.sem_flg = 0;
    while (1)
    {
        waitsem.sem_num = ASLEEPSEM;
        semop(semaphore, &waitsem, 1);
        waitsem.sem_num = SEATSARRAYSEM;
        semop(semaphore, &waitsem, 1); // czekanie na mozliwosc sprawdzenia tablicy klientow i ich ilosci
        printf("%d clients left.\n", shm->clientscounter);
        if (shm->clientscounter == 0)
        {
            unblocksem.sem_num = SEATSARRAYSEM;
            semop(semaphore, &unblocksem, 1); // odblokowuje modyfikacje tablicy klientow
            printf("%ld - GOLIBRODA: Falling asleep.\n", get_time());
            shm->asleep = 1;
            unblocksem.sem_num = ASLEEPSEM;
            semop(semaphore, &unblocksem, 1); //odblokowanie sprawdzania czy spi
            waitsem.sem_num = WAKEMEUPSEM;
            semop(semaphore, &waitsem, 1); // czekanie az ktos go obudzi
            shm->asleep = 0;
            unblocksem.sem_num = ASLEEPSEM;
            semop(semaphore, &unblocksem, 1); // klient zablokowal flage po dowiedzeniu sie ze spie wiec musze ja zmienic
            printf("%ld - GOLIBRODA: Waking up.\n", get_time());
            unblocksem.sem_num = SITSEM;
            semop(semaphore, &unblocksem, 1); // pozwalam usiasc
            waitsem.sem_num = SHAVEMESEM;
            semop(semaphore, &waitsem, 1); // czekam az usiadzie
            printf("%ld - GOLIBRODA: Shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            printf("%ld - GOLIBRODA: Ended shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            unblocksem.sem_num = SHAVINGENDEDSEM;
            semop(semaphore, &unblocksem, 1); // pozwalam odejsc
            waitsem.sem_num = ILEFTSEM;
            semop(semaphore, &waitsem, 1); // czekam az klient wyjdzie zeby zajac sie nastepnymi
        } else { // sa klienci
            printf("%ld - GOLIBRODA: Inviting client with PID %d to sit.\n", get_time(), shm->seats[0]);
            kill(shm->seats[0], SIGUSR1);
            unblocksem.sem_num = ASLEEPSEM;
            semop(semaphore, &unblocksem, 1); // odblokowanie sprawdzania czy spi bo i tak jest zajety
            unblocksem.sem_num = SITSEM;
            semop(semaphore, &unblocksem, 1); // pozwalam usiasc
            for (int i = 0; i <shm->clientscounter; ++i) shm->seats[i] = shm->seats[i+1]; // przesuniecie tablicy
            shm->clientscounter--; // usuniecie klienta
            unblocksem.sem_num = SEATSARRAYSEM;
            semop(semaphore, &unblocksem, 1); // mozna sprawdzac tablice klientow
            waitsem.sem_num = SHAVEMESEM;
            semop(semaphore, &waitsem, 1); // czekam az usiadzie
            printf("%ld - GOLIBRODA: Shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            printf("%ld - GOLIBRODA: Ended shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            unblocksem.sem_num = SHAVINGENDEDSEM;
            semop(semaphore, &unblocksem, 1); // skonczono strzyzenie, klient moze odejsc
            waitsem.sem_num = ILEFTSEM;
            semop(semaphore, &waitsem, 1); // czekam az klient wyjdzie zeby zajac sie nastepnymi
        }
    }
    return 0;
}
