#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int main(int argc, char *argv[]) {
    if (argc != 3)
    {
        kill(getppid(), SIGUSR1);
        FAILURE_EXIT(1, "Passed wrong number of arguments in child.\n");
    }
    char *pathname = argv[1];
    int N = (int) strtol(argv[2], NULL, 10);

    int fifo;
    if ((fifo = open(pathname, O_WRONLY)) < 0)
    {
        kill(getppid(), SIGUSR1);
        FAILURE_EXIT(1, "Couldn't open fifo file in slave.\n");
    }


    srand((uint ) ( time(NULL)* getpid()));
    printf("Moj PID: %d\n", getpid());
    FILE *date;
    size_t buffersize = 512;
    char buffer[2][buffersize];
    for (int i = 0; i <N ; ++i) {
        date = popen("date", "r");
        fgets(buffer[0], (int) buffersize, date);
        sprintf(buffer[1], "PID: %d, date: %s", getpid(), buffer[0]);
        write(fifo, buffer[1], strlen(buffer[1]));
        fclose(date);
        sleep((uint) rand()% 4 + 2);
    }
    close(fifo);
    return 0;
}
//
// Created by wojlewy on 14.04.18.
//

