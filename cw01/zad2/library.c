//
// Created by wojlewy on 11.03.18.
//

#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <values.h>

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
    char* closestblock;   // blok najblizszy rozmiarem
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