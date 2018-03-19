#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "operations.h"

void calctimediff (struct timeval tv1,struct timeval tv2)
{
    long secs = tv2.tv_sec - tv1.tv_sec;
    long msecs = tv2.tv_usec - tv1.tv_usec;
    if (msecs < 0) { msecs += 1000000; secs -= 1; }
    printf("time spent: %ld.%06ld", secs, msecs);
}


int main(int argc, char *argv[]) {

    if (argc<5)
    {
        printf("Pass at least 5 arguments");
        exit(1);
    }
    char *operation = argv[1];
    if(strcmp(operation, "generate") == 0)
    {
        char *filename = argv[2];
        long records = strtol(argv[3], NULL, 10);
        long length = strtol(argv[4], NULL, 10);
        if(records < 1 || length < 1)
        {
            printf("Wrong length of arguments. Pass numbers greater than 0");
            exit(1);
        }
        generate(filename, (size_t) records, (size_t) length);
    }
    else if(strcmp(operation, "sort") == 0)
    {
        char *filename = argv[2];
        long records = strtol(argv[3], NULL, 10);
        long length = strtol(argv[4], NULL, 10);
        char *opertype = argv[5];
        if(records < 1 || length < 1)
        {
            printf("Wrong length of arguments. Pass numbers greater than 0");
            exit(1);
        }
        if (strcmp(opertype, "sys") == 0)
        {
            struct timeval tv1,tv2;
            struct rusage usage1;
            struct timeval ustart1,uend1,sstart1,send1;
            getrusage(RUSAGE_SELF, &usage1);
            sstart1 = usage1.ru_stime;
            ustart1 = usage1.ru_utime;
            gettimeofday(&tv1, 0);
            syssort(filename, (size_t) records, (size_t) length);
            gettimeofday(&tv2, 0);
            getrusage(RUSAGE_SELF, &usage1);
            send1 = usage1.ru_stime;
            uend1 = usage1.ru_utime;
            printf( "real ");
            calctimediff(tv1, tv2);
            printf(" for system sort file\n");
            printf("user ");
            calctimediff(ustart1, uend1);
            printf(" for system sort file\n");
            printf("system ");
            calctimediff(sstart1, send1);
            printf(" for system sort file\n");
        }
        else if (strcmp(opertype, "lib") == 0)
        {
            struct timeval tv1,tv2;
            struct rusage usage1;
            struct timeval ustart1,uend1,sstart1,send1;
            getrusage(RUSAGE_SELF, &usage1);
            sstart1 = usage1.ru_stime;
            ustart1 = usage1.ru_utime;
            gettimeofday(&tv1, 0);
            libsort(filename, (size_t) records, (size_t) length);
            gettimeofday(&tv2, 0);
            getrusage(RUSAGE_SELF, &usage1);
            send1 = usage1.ru_stime;
            uend1 = usage1.ru_utime;
            printf( "real ");
            calctimediff(tv1, tv2);
            printf(" for lib sort file\n");
            printf("user ");
            calctimediff(ustart1, uend1);
            printf(" for lib sort file\n");
            printf("system ");
            calctimediff(sstart1, send1);
            printf(" for lib sort file\n");
        }
        else
        {
            printf("wrong type of operaration. Type \"sys\" or \"lib\".");
            exit(1);
        }
    }
    else if(strcmp(operation, "copy") == 0)
    {
        char *fromfile = argv[2];
        char *tofile = argv[3];
        long records = strtol(argv[4], NULL, 10);
        long buffersize = strtol(argv[5], NULL, 10);
        char *opertype = argv[6];
        if (strcmp(opertype, "sys") == 0)
        {
            struct timeval tv1,tv2;
            struct rusage usage1;
            struct timeval ustart1,uend1,sstart1,send1;
            getrusage(RUSAGE_SELF, &usage1);
            sstart1 = usage1.ru_stime;
            ustart1 = usage1.ru_utime;
            gettimeofday(&tv1, 0);
            syscopy(fromfile, tofile, (size_t) records, (size_t) buffersize);
            gettimeofday(&tv2, 0);
            getrusage(RUSAGE_SELF, &usage1);
            send1 = usage1.ru_stime;
            uend1 = usage1.ru_utime;
            printf( "real ");
            calctimediff(tv1, tv2);
            printf(" for sys copy file\n");
            printf("user ");
            calctimediff(ustart1, uend1);
            printf(" for sys copy file\n");
            printf("system ");
            calctimediff(sstart1, send1);
            printf(" for sys copy file\n");
        }
        else if (strcmp(opertype, "lib") == 0)
        {
            struct timeval tv1,tv2;
            struct rusage usage1;
            struct timeval ustart1,uend1,sstart1,send1;
            getrusage(RUSAGE_SELF, &usage1);
            sstart1 = usage1.ru_stime;
            ustart1 = usage1.ru_utime;
            gettimeofday(&tv1, 0);
            libcopy(fromfile, tofile, (size_t) records, (size_t) buffersize);
            gettimeofday(&tv2, 0);
            getrusage(RUSAGE_SELF, &usage1);
            send1 = usage1.ru_stime;
            uend1 = usage1.ru_utime;
            printf( "real ");
            calctimediff(tv1, tv2);
            printf(" for lib copy file\n");
            printf("user ");
            calctimediff(ustart1, uend1);
            printf(" for lib copy file\n");
            printf("system ");
            calctimediff(sstart1, send1);
            printf(" for lib copy file\n");
        }
        else
        {
            printf("wrong type of operaration. Type \"sys\" or \"lib\".");
            exit(1);
        }
    }
    else
    {
        printf("Wrong argument passed");
        exit(1);
    }
    return 0;
}