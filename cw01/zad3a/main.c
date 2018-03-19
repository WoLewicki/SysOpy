#include <dlfcn.h>

#ifndef DLL
#include "library.h"
#endif

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <memory.h>

void *handle;


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


int main(int argc, char* argv[]) {
    int next_option;
    const char *const short_options = "he:r:a:cs:d:m:"; //krotkie wersje polecen
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
    long arsize = 0; // wielkosc tablicy
    long blocksize = 0; // dlugosc bloku
    long alloctype = 0; // sposob alokacji pamieci
    long createarray = 0; // tworzenie tablicy
    long blocksearch = 0; // index bloku do przeszukiwania
    long deletenumber = 0; // liczba blokow do usuniecia
    long addnumber = 0; // liczba blokow do dodania
    program_name = argv[0]; // tu jest trzymana nazwa programu
    do {
        next_option = getopt_long(argc, argv, short_options,
                                  long_options, NULL);
        switch (next_option) {
            case 'h':
                print_usage(stdout, 0);
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
                print_usage(stderr, 1);
            case -1: // koniec opcji
                break;
            default: // cos niespodziewanego
                abort();
        }
    } while (next_option != -1);

#ifdef DLL

    char tab[100][100];
    handle = dlopen("./liblibrary.so", RTLD_LAZY);
    char **(*allocarray) (long, long, long, long, long) = dlsym(handle,"allocarray");
    void (*addblocks)(char **array, long addnumber, long blocksize, long arsize) = dlsym(handle,"addblocks");
    void (*deleteblocks)(char **array, long deletenumber, long arsize, long blocks) = dlsym(handle,"deleteblocks");
    void (*calctimediff) (struct timeval tv1,struct timeval tv2) = dlsym(handle,"calctimediff");
    void (*preparetab)() = dlsym(handle,"preparetab");

    char **(*makeArray)(long) = dlsym(handle,"makeArray");
    void (*deleteArray) (char**, long, long) = dlsym(handle,"deleteArray");
    char *(*addBlock)(char **, long, size_t) = dlsym(handle,"addBlock");
    void (*removeBlock)(char **, long) = dlsym(handle,"removeBlock");
    char *(*findClosestBlock)(char **, long, long) = dlsym(handle,"findClosestBlock");
    int (*sizeOfBlock)(char *) = dlsym(handle,"sizeOfBlock");
    int (*sizeOfStaticBlock) (long) = dlsym(handle,"sizeOfStaticBlock");
    int (*findClosestBlockInStatic) (long) = dlsym(handle,"findClosestBlockInStatic");

#endif
    if(alloctype == 0)
    {
    struct timeval tv1, tv2;
    struct rusage usage1;
    struct timeval ustart1, uend1, sstart1, send1;
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
    printf("real ");
    calctimediff(tv1, tv2);
    printf(" for alloc 1000 arrays and deleting it\n");
    printf("user ");
    calctimediff(ustart1, uend1);
    printf(" for alloc 1000 arrays and deleting it\n");
    printf("system ");
    calctimediff(sstart1, send1);
    printf(" for alloc 1000 arrays and deleting it\n");

    struct timeval tv3, tv4;
    struct timeval ustart2, uend2, sstart2, send2;
    struct rusage usage2;
    char **array = allocarray(arsize, blocksize, alloctype, createarray, addnumber);
    getrusage(RUSAGE_SELF, &usage2);
    sstart2 = usage2.ru_stime;
    ustart2 = usage2.ru_utime;
    gettimeofday(&tv3, 0);
    for (int i = 0; i < 1000; i++) {
        addblocks(array, addnumber, blocksize, arsize);
        deleteblocks(array, deletenumber, arsize, addnumber);
    }
    gettimeofday(&tv4, 0);
    getrusage(RUSAGE_SELF, &usage2);
    send2 = usage2.ru_stime;
    uend2 = usage2.ru_utime;
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

    struct timeval tv7, tv8;
    struct timeval ustart4, uend4, sstart4, send4;
    struct rusage usage4;
    char **array1 = allocarray(arsize, blocksize, alloctype, createarray, addnumber);
    strncpy(array1[0], "hello", strlen("hello"));
    strncpy(array1[1], "hellotoo", strlen("hellotoo"));
    strncpy(array1[2], "hellotooo", strlen("hellotooo"));

    getrusage(RUSAGE_SELF, &usage4);
    sstart4 = usage4.ru_stime;
    ustart4 = usage4.ru_utime;
    gettimeofday(&tv7, 0);
    if (addnumber <= arsize) {
        for (int i = 0; i < 1000; i++) {
            findClosestBlock(array1, addnumber, 1);
        }
    } else {
        for (int i = 0; i < 1000; i++) {
            findClosestBlock(array1, arsize, 1);
        }
    }
    gettimeofday(&tv8, 0);
    getrusage(RUSAGE_SELF, &usage4);
    send4 = usage4.ru_stime;
    uend4 = usage4.ru_utime;
    printf("real ");
    calctimediff(tv7, tv8);
    printf(" for finding block 1000 times\n");
    printf("user ");
    calctimediff(ustart4, uend4);
    printf(" for finding block 1000 times\n");
    printf("system ");
    calctimediff(sstart4, send4);
    printf(" for finding block 1000 times\n");
    deleteArray(array1, arsize, addnumber);
}
    if(alloctype == 1){
    preparetab();
    strncpy(tab[0], "hello", 100);
    strncpy(tab[1], "hellothere", 100);
    strncpy(tab[2], "hellotheregeneral", 100);
    strncpy(tab[3], "hellotheregeneralkenobi", 100);
    strncpy(tab[4], "hellotheregeneralkenoadffbi", 100);
    strncpy(tab[5], "hellotherfdsegeneralkffsenobi", 100);
    strncpy(tab[6], "hellotheregenefasdralkenobi", 100);
    struct timeval tv5, tv6;
    struct rusage usage3;
    struct timeval ustart3, uend3, sstart3, send3;
    getrusage(RUSAGE_SELF, &usage3);
    sstart3 = usage3.ru_stime;
    ustart3 = usage3.ru_utime;
    gettimeofday(&tv5, 0);
    for (int i = 0; i < 1000; i++) {
        findClosestBlockInStatic(blocksearch);
    }
    gettimeofday(&tv6, 0);
    getrusage(RUSAGE_SELF, &usage3);
    send3 = usage3.ru_stime;
    uend3 = usage3.ru_utime;
    printf("real ");
    calctimediff(tv5, tv6);
    printf(" for finding block 1000 times in static\n");
    printf("user ");
    calctimediff(ustart3, uend3);
    printf(" for finding block 1000 times in static\n");
    printf("system ");
    calctimediff(sstart3, send3);
    printf(" for finding block 1000 times in static\n");
}

#ifdef DLL
    dlclose(handle);
#endif
    return 0;
}
