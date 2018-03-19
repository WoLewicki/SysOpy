#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <memory.h>
#include "library.h"


const char* program_name; //nazwa programu
    void print_usage (FILE* stream, int exit_code)
    {
        fprintf (stream, "Usage: %s options [ inputfile zad2]\n", program_name);
        fprintf (stream,
                " -h --help            pokaz wszystkie opcje.\n"
                " -o --out             dokad ma zostac zapisany wynik.\n"
                " -e --elem            liczba elementow tablicy(tylko dla dynamicznej).\n"
                " -r --size            rozmiar bloku(tylko dla dynamicznej).\n"
                " -a --alloc           sposob alokacji pamieci, 0 - dyn, 1 - stat\n"
                " -c --create          stworzenie tablicy, 1 - tak, 0 - nie\n"
                " -s --search          wyszukanie elementu.\n"
                " -d --delete          usuniecie zadanej liczby elementow.\n"
                " -m --add             dodanie zadanej liczby elementow.\n");
        exit (exit_code);
    }
char **allocarray (long arsize, long blocksize, long alloctype, long createarray, long addnumber)
{
    if (createarray == 1 && alloctype == 0 && arsize != 0)
    {
        char **array = makeArray(arsize);
        if (blocksize > 0 && addnumber > 0)
        {
            for (int i=0;i<arsize && i<addnumber; i++)
            {
                array[i]= addBlock(array, i, (size_t) blocksize);
            }
        }
        return array;
    }
    if (createarray==1 && alloctype==1)
    {
        printf("przekazuje tablice 2D o wym [10][10] o nazwie tab");
    }
    return NULL;
}
void addblocks(char **array, long addnumber, long blocksize, long arsize)
{
    if (arsize>=addnumber) {
        for (int i = 0; i < addnumber; i++) {
            array[i] = addBlock(array, blocksize, sizeof(char));
        }
    }
}
void deleteblocks(char **array, long deletenumber, long arsize, long blocks)
{
    if (arsize>=deletenumber) {
        for (int i = 0; i < deletenumber&& i<blocks; i++) {
            removeBlock(array, i);
        }
    }
}
void calctimediff (struct timeval tv1,struct timeval tv2)
{
    long secs = tv2.tv_sec - tv1.tv_sec;
    long msecs = tv2.tv_usec - tv1.tv_usec;
    if (msecs < 0) { msecs += 1000000; secs -= 1; }
    printf("time spent: %ld.%06ld", secs, msecs);
}
void preparetab()
{
    for (int i=0;i<100;i++)
    {
        tab[i][0]='\0';
    }
}
void test1(long arsize, long blocksize, long alloctype, long createarray, long addnumber)
{
    struct timeval tv1,tv2;
    struct rusage usage1;
    struct timeval ustart1,uend1,sstart1,send1;
    getrusage(RUSAGE_SELF, &usage1);
    sstart1 = usage1.ru_stime;
    ustart1 = usage1.ru_utime;
    gettimeofday(&tv1, 0);
    for (int i = 0; i < 1000; i++) {
        char **array = allocarray(arsize, blocksize, alloctype, createarray, addnumber);
        deleteArray(array, arsize, addnumber);
    }
    gettimeofday(&tv2, 0);
    getrusage(RUSAGE_SELF, &usage1);
    send1 = usage1.ru_stime;
    uend1 = usage1.ru_utime;
    printf( "real ");
    calctimediff(tv1, tv2);
    printf(" for alloc 1000 arrays and deleting it\n");
    printf("user ");
    calctimediff(ustart1, uend1);
    printf(" for alloc 1000 arrays and deleting it\n");
    printf("system ");
    calctimediff(sstart1, send1);
    printf(" for alloc 1000 arrays and deleting it\n");
}
void test2(long addnumber, long blocksize, long arsize, long alloctype, long createarray, long deletenumber)
{
    struct timeval tv3,tv4;
    struct timeval ustart2,uend2,sstart2,send2;
    struct rusage usage1;
    char **array = allocarray(arsize, blocksize, alloctype, createarray, addnumber);
    getrusage(RUSAGE_SELF, &usage1);
    sstart2 = usage1.ru_stime;
    ustart2 = usage1.ru_utime;
    gettimeofday(&tv3, 0);
    for (int i = 0; i <1000 ; i++) {
        addblocks(array, addnumber, blocksize, arsize);
        deleteblocks(array, deletenumber, arsize, addnumber);
    }
    gettimeofday(&tv4, 0);
    getrusage(RUSAGE_SELF, &usage1);
    send2 = usage1.ru_stime;
    uend2 = usage1.ru_utime;
    printf("real ");
    calctimediff(tv3, tv4);
    printf(" for adding 1000 blocks and deleting them\n");
    printf("user ");
    calctimediff(ustart2, uend2);
    printf(" for adding 1000 blocks and deleting them\n");
    printf("system ");
    calctimediff(sstart2, send2);
    printf(" for adding 1000 blocks and deleting them\n");
    free(array);
}
void test3(long blocksearch)
{
    preparetab();
    strncpy(tab[0],"hello",100);
    strncpy(tab[1],"hellothere",100);
    strncpy(tab[2],"hellotheregeneral",100);
    strncpy(tab[3],"hellotheregeneralkenobi",100);
    strncpy(tab[4],"hellotheregeneralkenoadffbi",100);
    strncpy(tab[5],"hellotherfdsegeneralkffsenobi",100);
    strncpy(tab[6],"hellotheregenefasdralkenobi",100);
    struct timeval tv1,tv2;
    struct rusage usage1;
    struct timeval ustart1,uend1,sstart1,send1;
    getrusage(RUSAGE_SELF, &usage1);
    sstart1 = usage1.ru_stime;
    ustart1 = usage1.ru_utime;
    gettimeofday(&tv1, 0);
    for (int i=0;i<1000;i++)
    {
        findClosestBlockInStatic(blocksearch);
    }
    gettimeofday(&tv2, 0);
    getrusage(RUSAGE_SELF, &usage1);
    send1 = usage1.ru_stime;
    uend1 = usage1.ru_utime;
    printf( "real ");
    calctimediff(tv1, tv2);
    printf(" for finding block 1000 times in static\n");
    printf("user ");
    calctimediff(ustart1, uend1);
    printf(" for finding block 1000 times in static\n");
    printf("system ");
    calctimediff(sstart1, send1);
    printf(" for finding block 1000 times in static\n");
}
void test4(long addnumber, long blocksize, long arsize, long alloctype, long createarray)
{
    struct timeval tv3,tv4;
    struct timeval ustart2,uend2,sstart2,send2;
    struct rusage usage1;
    char **array = allocarray(arsize, blocksize, alloctype, createarray, addnumber);
    strncpy (array[0], "hello", strlen("hello"));
    strncpy (array[1], "hellotoo", strlen("hellotoo"));
    strncpy (array[2], "hellotooo", strlen("hellotooo"));

    getrusage(RUSAGE_SELF, &usage1);
    sstart2 = usage1.ru_stime;
    ustart2 = usage1.ru_utime;
    gettimeofday(&tv3, 0);
    if (addnumber<= arsize) {
        for (int i = 0; i < 1000; i++) {
            findClosestBlock(array, addnumber, 1);
        }
    }
    else {
        for (int i = 0; i < 1000; i++) {
            findClosestBlock(array, arsize, 1);
        }
    }
    gettimeofday(&tv4, 0);
    getrusage(RUSAGE_SELF, &usage1);
    send2 = usage1.ru_stime;
    uend2 = usage1.ru_utime;
    printf("real ");
    calctimediff(tv3, tv4);
    printf(" for finding block 1000 times\n");
    printf("user ");
    calctimediff(ustart2, uend2);
    printf(" for finding block 1000 times\n");
    printf("system ");
    calctimediff(sstart2, send2);
    printf(" for finding block 1000 times\n");
    deleteArray(array, arsize, addnumber);
}
    int main(int argc, char* argv[])
    {
     int next_option;
     const char* const short_options = "he:r:a:cs:d:m:"; //krotkie wersje polecen
        const struct option long_options[] = { //dlugie wersje polecen
                {"help",   0, NULL, 'h'},
                {"elem",   1, NULL, 'e'},
                {"size",   1, NULL, 'r'},
                {"alloc",  1, NULL, 'a'},
                {"create", 0, NULL, 'c'},
                {"search", 1, NULL, 's'},
                {"delete", 1, NULL, 'd'},
                {"add",    1, NULL, 'm'},
                {NULL,     0, NULL, 0}
        };
        long arsize=0; // wielkosc tablicy
        long blocksize=0; // dlugosc bloku
        long alloctype = 0; // sposob alokacji pamieci
        long createarray = 0; // tworzenie tablicy
        long blocksearch=0; // index bloku do przeszukiwania
        long deletenumber=0; // liczba blokow do usuniecia
        long addnumber=0; // liczba blokow do dodania
        program_name = argv[0]; // tu jest trzymana nazwa programu
        do {
            next_option = getopt_long (argc, argv, short_options,
                                       long_options, NULL);
            switch (next_option)
            {
                case 'h':
                    print_usage (stdout, 0);
                case 'e':
                    arsize = strtol(optarg, NULL, 10);
                    break;
                case 'r':
                    blocksize = strtol(optarg, NULL, 10);
                    break;
                case 'a':
                    alloctype = strtol(optarg, NULL, 10);
                    break;
                case 'c':
                    createarray = 1;
                    break;
                case 's':
                    blocksearch = strtol(optarg, NULL, 10);
                    break;
                case 'd':
                    deletenumber = strtol(optarg, NULL, 10);
                    break;
                case 'm':
                    addnumber = strtol(optarg, NULL, 10);
                    break;
                case '?': // podano zla opcje, wypisz usage na stderr i wyjdz przy pomocy exit code one (nienormalne zakonczenie)
                    print_usage (stderr, 1);
                case -1: // koniec opcji
                    break;
                default: // cos niespodziewanego
                    abort ();
            }
        }
        while (next_option != -1);

        if (alloctype==0) {
            test1(arsize, blocksize, alloctype, createarray, addnumber);
            test2(addnumber, blocksize, arsize, alloctype, createarray, deletenumber);
            test4(addnumber, blocksize, arsize, alloctype, createarray);
        }
            if (alloctype ==1) test3(blocksearch);

        return 0;
}