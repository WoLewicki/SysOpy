#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <bits/siginfo.h>
#include <wait.h>
#include <errno.h>

volatile int requestcounter =0;
volatile int M;
volatile int N;
volatile int remainingprocesses;
pid_t *processarray;
pid_t *waitingarray;

void printreceived (int signo, siginfo_t *info, void *context)
{
    printf("Otrzymano sygnal SIGRTMIN+%d od procesu o pid: %d\n", signo-SIGRTMIN, info->si_pid);
}


void addrequest (int signo, siginfo_t *info, void *context) {
    printf("Otrzymano sygnal SIGUSR1 od pid %d.\n", info->si_pid);
    if (requestcounter < M) {
        waitingarray[requestcounter++] = info->si_pid;
        printf("Dodaje zadanie rozpoczecia. Wyslano: %d, potrzeba jescze: %d\n", requestcounter, M - requestcounter);
        if (requestcounter >= M) {
            printf("Przekroczono prog prosb o rozpoczecie.\n");
            for (int i = 0; i < requestcounter; ++i) {
                if (waitingarray[i] > 0) {
                    printf("Pozwalam wyslac SIGRT procesowi o pid %d i czekam na wynik jego dzialania.\n",
                           waitingarray[i]);
                    kill(waitingarray[i], SIGUSR1);
                    waitpid(waitingarray[i], NULL, 0);
                    remainingprocesses--;
                }
            }
        }
    } else {
        printf("Pozwalam od razu wyslac SIGRT procesowi o pid %d i czekam na wynik jego dzialania.\n",
               info->si_pid);
        kill(info->si_pid, SIGUSR1);
        waitpid(info->si_pid, NULL, 0);
        remainingprocesses--;
    }

}

void killhandler (int signo, siginfo_t *info, void *context)
{
    int counter =0;
    printf("Otrzymano sygnal SIGINT. Zabijanie wszystkich procesow.\n");
    for (int i=0;i<N; i++)
    {
        if (processarray[i] > 0)
        {
            if(kill(processarray[i], SIGKILL) == -1)
                counter++;
            waitpid(processarray[i], NULL, 0);
        }
    }
    printf("Zabito wszystkie procesy. %d skonczylo sie przed terminacja.\n", counter);
    exit(0);
}

void childhandler (int signo, siginfo_t *info, void *context)
{
        printf("Proces o pid %d powrocil z wartoscia %d.\n", info->si_pid, info->si_status);
    if (remainingprocesses == 0)
        {
            printf("Wszystkie procesy sie zakonczyly. Konczenie programu.\n");
            free(waitingarray);
            free(processarray);
            exit(0);
    }
}


int main(int argc, char *argv[]) {

    if (argc != 3)
    {
        printf("Zla liczba argumentow.\n");
        exit(1);
    }
    N = (int) strtol(argv[1], NULL, 10);
    M = (int) strtol(argv[2], NULL, 10);
    if (N<M || N<1 || M<0)
    {
        printf("Podano zle argumenty.\n");
        exit(1);
    }
    processarray = calloc((size_t )N, sizeof(int));
    waitingarray = calloc((size_t )N, sizeof(int));
    pid_t process;
    remainingprocesses = N;

    // obsluga sygnalow

    struct sigaction act;
    act.sa_sigaction = childhandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_NODEFER | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &act, NULL) == -1)
    {
        printf("Nie dalo sie zlapac SIGCHLD.\n");
        exit(1);
    }
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = killhandler;
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        printf("Nie dalo sie zlapac SIGINT.\n");
        exit(1);
    }

    act.sa_sigaction = addrequest;
    if (sigaction(SIGUSR1, &act, NULL) == -1)
    {
        printf("Nie dalo sie zlapac SIGUSR1 .\n");
        exit(1);
    }

    act.sa_sigaction = printreceived;

    for (int i = SIGRTMIN; i<= SIGRTMAX; i++) { // lapanie real time signals
        if (sigaction(i, &act, NULL) == -1)
        {
            printf("Nie dalo sie zlapac SIGRTMIN+%d.\n",i - SIGRTMIN );
            exit(1);
        }
    }


    for (int i=0; i<N; i++)
    {
        usleep(26210);
        process = fork();
        if (process < 0) {
            printf("Nie dalo sie stworzyc procesu.\n");
            exit(1);
        }

        if (process)
        {
                printf("Tworzenie procesu potomnego o ID: %d.\n", process);
                processarray[i] = process;
        }
        else
        {
            if(execl("./child", "./child", NULL) <0)
                printf("Nie dalo sie uruchomic programu.\n");
            exit(1);
        }
    }
    while (wait(NULL)); // when there are no child processes, it returns -1 and quits while

    return 0;
}