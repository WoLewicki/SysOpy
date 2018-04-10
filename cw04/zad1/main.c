#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

volatile int state = 0;

void handleprogram(int signo)
{
    if (signo == SIGTSTP) {
        if (state == 0) {
        	printf("\n");
            printf("Oczekuje na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
            state = 1;
        } else if (state == 1) {
            state = 0;
        }
    }
    else if (signo == SIGINT)
        {
        		printf("\n");
                printf("Odebrano sygnal SIGINT.\n");
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

int main() {
    signal(SIGINT, handleprogram);
    struct sigaction act1;
    act1.sa_handler = handleprogram;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = 0;
    sigaction(SIGTSTP, &act1, NULL);

    while(1)
    {
            printtime();
            sleep(1);
        if (state == 1) pause();
    }
    return 0;
}
