#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <wait.h>

int process;
int myflag=0;

void handleprogram(int signo)
{
    if (signo == SIGTSTP) {
    	printf("\n");
        printf("Odebrano sygnal SIGTSTP. ");
        if (!myflag)
        {
        	printf("Zabijanie skryptu i oczekiwanie na CTRL+C - kontynuacja, CTRL+Z - zakonczenie programu.\n");
            myflag = 1;
            if (kill(process, SIGKILL) != 0) printf("Nie zabito procesu z procesu macierzystego po wyslaniu SIGTSTP.\n");
        }
        else
        {  
        	printf("Wznawianie dzialania skryptu.\n");
        	myflag =0;
        }
    }
    else if (signo == SIGINT)
    {
    	printf("\n");
        printf("Odebrano sygnal SIGINT: konczenie programu.\n");
        if (kill(process, SIGKILL) != 0) printf("Nie zabito procesu przed zakonczeniem programu, poniewaz nie byl uruchomiony.\n");
        _exit(0);
    }
    else {
    	printf("\n");
        printf("Wrong signal detected.\n");
        _exit(1);
    }
}

void printtime ()
{
    time_t timer;
    struct tm *currenthour;

    timer = time(NULL);
    currenthour = localtime(&timer);
    printf("%d:%d:%d\n", currenthour->tm_hour, currenthour->tm_min, currenthour->tm_sec);
}

int main(int argc, char *argv[]) {

    char *args[] = {"sh", "./dateprinter.sh", NULL};
    int status;
    signal(SIGINT, handleprogram);
    struct sigaction act1;
    act1.sa_handler = handleprogram;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = 0;
    sigaction(SIGTSTP, &act1, NULL);

    while (1) {
        if(!myflag) {
            process = (int) fork();
            if (!process) {
                if (execvp(args[0], args) == -1) {
                    printf("Nie wykonano polenia.\n");
                    _exit(1);
                }
            } else {

                waitpid(process, &status, 0);
                if (status != 0)
                {
                    printf("Something gone wrong while trying to end a process.\n");
                    exit(1);
                }
            }
        }
    }
    return 0;
}//
// Created by wojlewy on 02.04.18.
//

