#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int masterflag = 0;

void usrhandler (int signo, siginfo_t *info, void* context)
{
    if (signo == SIGUSR1)
    {
        printf("Something went wrong in process of PID: %d. Ending program.\n", info->si_pid);
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    if (argc != 4) FAILURE_EXIT(1, "Pass 3 arguments to main.(fifo pathname, N and number of child processes)\n");

    char *pathname = argv[1];
    int childnumber = (int) strtol(argv[3], NULL, 10);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = usrhandler;
    if(sigaction(SIGUSR1, &act, NULL) < 0) FAILURE_EXIT(1, "Couldn't catch SIGUSR1 in main.\n");
    pid_t master = fork();
    if (master < 0) FAILURE_EXIT(1, "Couldn't make master.\n");
    if (!master) //dziecko
    {
        execlp("./master", "./master", pathname, NULL);
        FAILURE_EXIT(1, "Couldn't exec master.\n");
    }
    sleep(1); // daje masterowi czas na otwarcie fifo

    for (int i = 0; i <childnumber; ++i) {
        printf("Forking %d slave.\n", i+1);
        pid_t slave = fork();
        if (slave < 0) FAILURE_EXIT(1, "Couldn't fork a slave.\n");
        if (!slave)
        {
            execlp("./slave", "./slave", argv[1], argv[2], NULL);
            FAILURE_EXIT(1, "Couldn't exec one of slaves.\n");
        }
    }

    while(wait(NULL)) // czekam az wszystkie dzieci sie zakoncza
        if (errno == ECHILD)
        {
            printf("\n");
            break;
        }
    return 0;
}