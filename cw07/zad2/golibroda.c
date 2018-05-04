#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "specifications.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int sharedmem = -1;
sem_t *seatsarraysem;
sem_t *asleepsem;
sem_t *wakemeupsem;
sem_t *sitsem;
sem_t *shavemesem;
sem_t *shavingendedsem;
sem_t *ileftsem;
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
    if(shm_unlink(shmpath) < 0) printf("GOLIBRODA: Something went wrong while deleting shm.\n");
    if(sem_close(seatsarraysem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(asleepsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(wakemeupsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(sitsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(shavemesem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(shavingendedsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(ileftsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_unlink(seatsarray) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_unlink(isasleep) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_unlink(wakemeup) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_unlink(sit) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_unlink(shaveme) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_unlink(shavingended) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_unlink(ileft) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");

}

int main(int argc, char *argv[]) {
    if (atexit(removeshmandsem) < 0) FAILURE_EXIT(1, "GOLIBRODA: Couldn't register atexit function.\n");
    if (argc != 2 ) FAILURE_EXIT(1, "GOLIBRODA: Pass number of seats.\n");
    int seatlimit = (int) strtol(argv[1], NULL, 10);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    if ((seatsarraysem = sem_open(seatsarray, O_CREAT | 0666, DEFFILEMODE, 1)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");
    if ((asleepsem = sem_open(isasleep, O_CREAT | 0666, DEFFILEMODE, 1)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");
    if ((wakemeupsem = sem_open(wakemeup, O_CREAT | 0666, DEFFILEMODE, 0)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");
    if ((sitsem = sem_open(sit, O_CREAT | 0666, DEFFILEMODE, 0)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");
    if ((shavemesem = sem_open(shaveme, O_CREAT | 0666, DEFFILEMODE, 0)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");
    if ((shavingendedsem = sem_open(shavingended, O_CREAT | 0666, DEFFILEMODE, 0)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");
    if ((ileftsem = sem_open(ileft, O_CREAT | 0666, DEFFILEMODE, 0)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make semaphore.\n");

    if ((sharedmem = shm_open(shmpath, O_CREAT | O_RDWR | O_TRUNC, DEFFILEMODE)) < 0) FAILURE_EXIT(1, "GOLIBRODA:Couldn't make shmem.\n");
    ftruncate(sharedmem, sizeof(Shm));
    if ((shm = (Shm *) mmap(NULL, sizeof(Shm), PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem, 0)) < 0)
        FAILURE_EXIT(1, "GOLIBRODA: Couldn't get pointer to shm.\n");
    shm->seatlimit = seatlimit;
    shm->clientscounter = 0;
    for (int i = 0; i <seatlimit ; ++i) {
        shm->seats[i] = 0;
    }

    while (1)
    {
        sem_wait(asleepsem);
        printf("Tuzem doszedl.\n");
        sem_wait(seatsarraysem);
        printf("%d clients left.\n", shm->clientscounter);
        if (shm->clientscounter == 0)
        {
            sem_post(seatsarraysem);
            printf("%ld - GOLIBRODA: Falling asleep.\n", get_time());
            shm->asleep = 1;
            sem_post(asleepsem);
            sem_wait(wakemeupsem);
            shm->asleep = 0;
            sem_post(asleepsem);
            printf("%ld - GOLIBRODA: Waking up.\n", get_time());
            sem_post(sitsem);
            sem_wait(shavemesem);
            printf("%ld - GOLIBRODA: Shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            printf("%ld - GOLIBRODA: Ended shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            sem_post(shavingendedsem);
            sem_wait(ileftsem);
        } else { // sa klienci
            printf("%ld - GOLIBRODA: Inviting client with PID %d to sit.\n", get_time(), shm->seats[0]);
            kill(shm->seats[0], SIGUSR1);
            sem_post(asleepsem);
            sem_post(sitsem);
            for (int i = 0; i <shm->clientscounter; ++i) shm->seats[i] = shm->seats[i+1]; // przesuniecie tablicy
            shm->clientscounter--; // usuniecie klienta
            sem_post(seatsarraysem);
            sem_wait(shavemesem);
            printf("%ld - GOLIBRODA: Shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            printf("%ld - GOLIBRODA: Ended shaving client with PID: %d\n", get_time(), shm->currentlyshaved);
            sem_post(shavingendedsem);
            sem_wait(ileftsem);
        }
    }
    return 0;
}