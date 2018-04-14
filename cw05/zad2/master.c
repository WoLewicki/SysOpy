#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>


#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}


int main(int argc, char *argv[]) {
    if (argc != 2) FAILURE_EXIT(1, "Pass 1 arguments to master.\n");
    FILE *fifo;
    if (mkfifo(argv[1], S_IRUSR | S_IWUSR) == -1)
    {
        kill(getppid(), SIGUSR1);
        FAILURE_EXIT(1, "Couldn't make fifo in master.\n");
    }
    if ((fifo = fopen(argv[1], "r")) == NULL) // po tym proces blokuje sie az do otwarcia drugiego konca fifo
    {
        kill(getppid(), SIGUSR1);
        FAILURE_EXIT(1, "Couldn't open fifo file in master.\n");
    }


    size_t buffersize = 512;
    char reader[buffersize];
    while(fgets(reader, (int) buffersize, fifo)) write(STDOUT_FILENO, reader, strlen(reader));
    printf("No more lines to read. Closing master.\n");
    fclose(fifo);
    if (remove(argv[1]) != 0) {
        kill(getppid(), SIGUSR1);
        FAILURE_EXIT(1, "Couldn't remove fifo in master.\n");
    }

    return 0;
}



//
// Created by wojlewy on 14.04.18.
//

