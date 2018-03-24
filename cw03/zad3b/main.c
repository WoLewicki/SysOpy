#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#define ARGS_MAX 12
#define LINE_MAX 120

void calctimediff (struct timeval tv1,struct timeval tv2)
{
    long secs = tv2.tv_sec - tv1.tv_sec;
    long msecs = tv2.tv_usec - tv1.tv_usec;
    if (msecs < 0) { msecs += 1000000; secs -= 1; }
    printf("time spent: %ld.%06ld", secs, msecs);
}

int main(int argc, char *argv[]) {
    if (argc != 4)
    {
        printf("Wrong number of arguments. Pass 3.\n");
        exit(1);
    }
    char *filename = argv[1];
    float cpulimit = strtof(argv[2], NULL);
    int datalimit = (int) strtol(argv[3], NULL, 10);
    datalimit = datalimit * 1048576; // convert from MB to bytes
    FILE *input = fopen(filename, "r");
    if (input == NULL)
    {
        printf("Couldn't open file.\n");
        exit(EXIT_FAILURE);
    }
    size_t len = 0;
    char *line = calloc(LINE_MAX, sizeof(char));
    while (getline(&line, &len, input) != -1) {
        char *word;
        word = strtok (line," \n");
        int i = 0;
        char **arguments = calloc(ARGS_MAX, sizeof(char*)); // przyjmuje do ARGS_MAX argumentow
        while (line !=NULL && word !=NULL && i<ARGS_MAX)
        {
            arguments[i] = word;
            word = strtok (NULL, " \n\t");
            i++;
        }
        int processresult;
        struct timeval tv1,tv2;
        struct rusage usage1;
        struct timeval ustart1,uend1,sstart1,send1;
        getrusage(RUSAGE_CHILDREN, &usage1);
        sstart1 = usage1.ru_stime;
        ustart1 = usage1.ru_utime;
        gettimeofday(&tv1, 0);
        pid_t process = vfork();
        if (process == 0)
        {
            if (strcmp(arguments[0], "0") == 0)
            {
                printf("Didn't pass function name.\n");
                exit(1);
            }
            printf("\n\n");
            printf("Executing function: %s in process of pid : %d with arguments : ",arguments[0], getpid());
            for (int j = 0; j <i ; ++j) {
                printf("%s ",arguments[j]);
            }
            printf("\n");
            struct rlimit limits = {(rlim_t) cpulimit, (rlim_t) (cpulimit +1) };
            if((setrlimit(RLIMIT_CPU, &limits))<0)
            {
                printf("Coudln't change process resources for CPU.");
            }
            struct rlimit limits1= {(rlim_t) datalimit, (rlim_t) (datalimit+ 1000)};
            if((setrlimit(RLIMIT_DATA, &limits1))<0)
            {
                printf("Coudln't change process resources for DATA.");
            }
            if((setrlimit(RLIMIT_STACK, &limits1))<0)
            {
                printf("Coudln't change process resources for STACK.");
            }

            execvp(arguments[0], arguments);

        }
        else {
            if (process != wait(&processresult)) {
                printf("Something went wrong when waiting for the result.\n");
                exit(1);
            }
            gettimeofday(&tv2, 0);
            getrusage(RUSAGE_CHILDREN, &usage1);
            send1 = usage1.ru_stime;
            uend1 = usage1.ru_utime;
            printf("\n");
            printf( "real ");
            calctimediff(tv1, tv2);
            printf(" used on this function\n");
            printf("user ");
            calctimediff(ustart1, uend1);
            printf(" used on this function\n");
            printf("system ");
            calctimediff(sstart1, send1);
            printf(" used on this function\n");
            if (processresult != 0) {
                printf("Ended with signal: %d\n",processresult);
                printf("Something gone wrong in command : %s. Maybe you didnt pass right file/arguments/sysfunction\n",
                       arguments[0]);
                exit(1);
            }
            free(arguments);
        }
    }
    fclose(input);
    return 0;
}
