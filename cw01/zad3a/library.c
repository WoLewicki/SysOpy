//
// Created by wojlewy on 11.03.18.
//

#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <memory.h>

char **makeArray(long n)
{
    return calloc(n,sizeof(char*));
}
void deleteArray (char** array, long n, long blocks) {
    for (long i = 0; i < n&& i< blocks; i++) {
        removeBlock(array, i);
    }
    free(array);
}
char *addBlock(char **array, long n, size_t size) {
    array[n] =  calloc(size, sizeof(char));
    return array[n];
}
void removeBlock(char **array, long i) {
    free(array[i]);
}
int sizeOfBlock(char *block)
{
    char *c = block;
    int size = 0;
    while(*c != '\0')
    {
        size += (int) *c;
        c++;
    }
    return size;
}

char *findClosestBlock(char **array, long n, long blockindex) {
    int blocktocompare = sizeOfBlock(array[blockindex]); // wart. ascii bloku z ktorym porownojemy inne
    char* closestblock = array[0];   // blok najblizszy rozmiarem
    int diffbetweenblocks = INT_MAX; // roznica kodow ascii porownywanego bloku i najblizszego rozmiarem bloku

    for (int i = 0; i < n; i++) {
        if (i != blockindex) {
            if ( abs(sizeOfBlock(array[i]) - blocktocompare) < diffbetweenblocks) {
                diffbetweenblocks = abs(sizeOfBlock(array[i]) - blocktocompare);
                closestblock = array[i];
            }
        }
    }
    return closestblock;
}

int sizeOfStaticBlock (long n)
{
    if (n<100) {
        int size = 0;
        for (int i = 0; tab[n][i] != '\0' && i < 100; i++) {
            size += (int) tab[n][i];
        }
        return size;
    }
    return 0;
}
int findClosestBlockInStatic (long blockindex)
{
    if (blockindex<100 && blockindex>-1) {
        long blocktocompare = sizeOfStaticBlock(blockindex); // wart. ascii wiersza, z ktorym porownojemy inne
        int indexOfClosestBlock =0;   // index wiersza najblizszego rozmiarem
        long diffbetweenblocks = INT_MAX; // roznica kodow ascii porownywanego wiersza i najblizszego rozmiarem wiersza

        for (int i = 0; i < 100; i++) {
            if (i != blockindex && *tab[i] != '\0') {
                if (labs(sizeOfStaticBlock(i) - blocktocompare) < diffbetweenblocks) {
                    diffbetweenblocks = labs(sizeOfStaticBlock(i) - blocktocompare);
                    indexOfClosestBlock = i;
                }
            }
        }
        return indexOfClosestBlock;
    }
    return 0;
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
