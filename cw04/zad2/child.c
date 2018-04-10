#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void receiver(int signo){
kill(getppid(), SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN)));
}

int main()
{
 			signal(SIGUSR1, receiver); // odebranie prosby o pozwolenie do dzialania
            sigset_t mask;
            sigfillset(&mask);
            sigdelset(&mask, SIGUSR1); // blokowanie wszystkich sygnalow oprocz SIGUSR1
            uint sleeptimer;
            srand((uint)(getppid() * getpid()));
            sleeptimer = (uint)rand()%11;
            sleep(sleeptimer);
            kill(getppid(), SIGUSR1); // wyslanie prosby o dzialanie
            sigsuspend(&mask); // czekanie na SIGUSR1
            return sleeptimer;
}
