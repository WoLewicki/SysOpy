#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>
#include "specifications.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int flag = 0;
Shm *shm;

long get_time(){
    long timer;
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    timer = t.tv_nsec / 1000;
    return timer;
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
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGUSR1, flagchanger);
    if (argc != 3) FAILURE_EXIT(1, "Pass 2 arguments.\n");
    int members = (int) strtol(argv[1], NULL, 10);
    int shavesnumber = (int) strtol(argv[2], NULL, 10);
    key_t semakey = ftok(semaphorepath, PROJ_ID);
    int semaphore;
    if ((semaphore= semget(semakey, 0, 0666)) < 0) FAILURE_EXIT(1, "KLIENT:Couldn't make semaphore.\n");

    key_t shmkey = ftok(shmpath, PROJ_ID);
    int sharedmem;
    if ((sharedmem = shmget(shmkey, sizeof(Shm), 0666)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get shmem.\n");
    if ((shm = (Shm *) shmat(sharedmem, NULL, 0)) < 0) FAILURE_EXIT(1, "KLIENT: Couldn't get pointer to shm.\n");

    pid_t processarray[1000];
    int l = 0;

    struct sembuf waitsem;
    waitsem.sem_op = -1;
    waitsem.sem_flg = 0;
    struct sembuf unblocksem;
    unblocksem.sem_op = 1;
    unblocksem.sem_flg = 0;

    for (int i = 0; i <members; ++i) {
        pid_t child = fork();
        if (child == 0)
        {
            for (int j = 0; j < shavesnumber; ++j) {
                waitsem.sem_num = ASLEEPSEM;
                semop(semaphore, &waitsem, 1); // czekam na zmiane flagi spania
                if (shm->asleep == 1)
                {
                    shm->currentlyshaved = getpid();

                    printf("%ld - KLIENT %d: Waking up the barber.\n", get_time(), getpid());
                    unblocksem.sem_num = WAKEMEUPSEM;
                    semop(semaphore, &unblocksem, 1); // obudzenie golibrody
                    waitsem.sem_num = SITSEM;
                    semop(semaphore, &waitsem, 1); // czekanie az bedzie mogl usiasc
                    printf("%ld - KLIENT %d: Sitting on barber's chair immidiately.\n", get_time(), getpid());
                    unblocksem.sem_num = SHAVEMESEM;
                    semop(semaphore, &unblocksem, 1); // pozwalam sie strzyc
                    waitsem.sem_num = SHAVINGENDEDSEM;
                    semop(semaphore, &waitsem, 1); // czekam az skonczy
                    printf("%ld - KLIENT %d: Leaving after being shaved for %d time.\n", get_time(), getpid(), j+1);
                    unblocksem.sem_num = ILEFTSEM;
                    semop(semaphore, &unblocksem, 1); // daje znac ze wyszedlem
                } else // golibroda nie spi wiec strzyze klienta
                {
                    waitsem.sem_num = SEATSARRAYSEM;
                    semop(semaphore, &waitsem, 1); // czekam az bede mogl modyfikowac tablice
                    if (shm->clientscounter >= shm->seatlimit) { // nie ma miejsc
                        printf("%ld - KLIENT %d: Leaving because there are no free seats.\n", get_time(), getpid());
                        unblocksem.sem_num = ASLEEPSEM;
                        semop(semaphore, &unblocksem, 1);
                        unblocksem.sem_num = SEATSARRAYSEM;
                        semop(semaphore, &unblocksem, 1); // mozna operowac na tablicy klientow
                    } else { // ustawienie sie w kolejce i inkrementowanie liczby klientow
                        printf("%ld - KLIENT %d: Waiting in the queue because barber is busy.\n", get_time(), getpid());
                        shm->seats[shm->clientscounter] =  getpid();
                        shm->clientscounter++;
                        unblocksem.sem_num = ASLEEPSEM;
                        semop(semaphore, &unblocksem, 1); // skoro golibroda strzyze to nie odblokuje mi semafora wiec sam musze pozwolic innym sprawdzac
                        unblocksem.sem_num = SEATSARRAYSEM;
                        semop(semaphore, &unblocksem, 1); // mozna operowac na tablicy klientow
                        while (!flag); // czekanie az bd pierwszym klientem w kolejce
                        flag = 0;
                        waitsem.sem_num = SITSEM;
                        semop(semaphore, &waitsem, 1); // czekanie az zostanie zaproszony na krzeslo
                        printf("%ld - KLIENT %d: Sitting on barber's chair.\n", get_time(), getpid());
                        shm->currentlyshaved = getpid();
                        unblocksem.sem_num = SHAVEMESEM;
                        semop(semaphore, &unblocksem, 1);
                        waitsem.sem_num = SHAVINGENDEDSEM;
                        semop(semaphore, &waitsem, 1);
                        printf("%ld - KLIENT %d: Leaving after being shaved for %d time.\n", get_time(), getpid(), j+1);
                        unblocksem.sem_num = ILEFTSEM;
                        semop(semaphore, &unblocksem, 1); // daje znac ze wyszedlem
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

