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
#include <wait.h>

#include "specifications.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int flag = 0;
Shm *shm;

sem_t *seatsarraysem;
sem_t *asleepsem;
sem_t *wakemeupsem;
sem_t *sitsem;
sem_t *shavemesem;
sem_t *shavingendedsem;
sem_t *ileftsem;

long get_time(){
    long timer;
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    timer = t.tv_nsec / 1000;
    return timer;
}

void removesem ()
{
    if(sem_close(seatsarraysem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(asleepsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(wakemeupsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(sitsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(shavemesem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(shavingendedsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
    if(sem_close(ileftsem) < 0) printf("GOLIBRODA: Something went wrong while deleting semaphores.\n");
}

void flagchanger (int signo)
{
    if (signo == SIGUSR1) flag = 1;
}
void sighandler (int signo)
{
    exit(0);
}



int main(int argc, char *argv[]) {
    if (atexit(removesem) < 0) FAILURE_EXIT(1, "GOLIBRODA: Couldn't register atexit function.\n");
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGUSR1, flagchanger);
    if (argc != 3) FAILURE_EXIT(1, "Pass 2 arguments.\n");
    int members = (int) strtol(argv[1], NULL, 10);
    int shavesnumber = (int) strtol(argv[2], NULL, 10);

    if ((seatsarraysem = sem_open(seatsarray, 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get semaphore.\n");
    if ((asleepsem = sem_open(isasleep, 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get semaphore.\n");
    if ((wakemeupsem = sem_open(wakemeup, 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get semaphore.\n");
    if ((sitsem = sem_open(sit, 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get semaphore.\n");
    if ((shavemesem = sem_open(shaveme, 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get semaphore.\n");
    if ((shavingendedsem = sem_open(shavingended, 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get semaphore.\n");
    if ((ileftsem = sem_open(ileft, 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get semaphore.\n");

    int sharedmem;
    if ((sharedmem = shm_open(shmpath, 0666, DEFFILEMODE)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get shmem.\n");
    if ((shm = (Shm *) mmap(NULL, sizeof(Shm), PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem, 0)) < 0)
        FAILURE_EXIT(1, "KLIENT: Couldn't get pointer to shm.\n");

    pid_t processarray[1000];
    int l = 0;

    for (int i = 0; i <members; ++i) {
        pid_t child = fork();
        if (child == 0)
        {
            for (int j = 0; j < shavesnumber; ++j) {
                sem_wait(asleepsem);
                if (shm->asleep == 1)
                {
                    shm->currentlyshaved = getpid();

                    printf("%ld - KLIENT %d: Waking up the barber.\n", get_time(), getpid());
                    sem_post(wakemeupsem);
                    sem_wait(sitsem);
                    printf("%ld - KLIENT %d: Sitting on barber's chair immidiately.\n", get_time(), getpid());
                    sem_post(shavemesem);
                    sem_wait(shavingendedsem);
                    printf("%ld - KLIENT %d: Leaving after being shaved for %d time.\n", get_time(), getpid(), j+1);
                    sem_post(ileftsem);
                } else // golibroda nie spi wiec strzyze klienta
                {
                    sem_wait(seatsarraysem);
                    if (shm->clientscounter >= shm->seatlimit) { // nie ma miejsc
                        printf("%ld - KLIENT %d: Leaving because there are no free seats.\n", get_time(), getpid());
                        sem_post(asleepsem);
                        sem_post(seatsarraysem);
                    } else { // ustawienie sie w kolejce i inkrementowanie liczby klientow
                        printf("%ld - KLIENT %d: Waiting in the queue because barber is busy.\n", get_time(), getpid());
                        shm->seats[shm->clientscounter] =  getpid();
                        shm->clientscounter++;
                        sem_post(asleepsem);
                        sem_post(seatsarraysem);
                        while (!flag); // czekanie az bd pierwszym klientem w kolejce
                        flag = 0;
                        sem_wait(sitsem);
                        printf("%ld - KLIENT %d: Sitting on barber's chair.\n", get_time(), getpid());
                        shm->currentlyshaved = getpid();
                        sem_post(shavemesem);
                        sem_wait(shavingendedsem);
                        printf("%ld - KLIENT %d: Leaving after being shaved for %d time.\n", get_time(), getpid(), j+1);
                        sem_post(ileftsem);
                    }
                }
            }
            return 0;
        } else
        {
            processarray[l] = child;
            l++;
        }
    }
    for (int k = 0; k <l ; ++k) {
        waitpid(processarray[k], NULL, 0);
    }

    return 0;
}//
// Created by wojlewy on 26.04.18.
//
