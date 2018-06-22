#include "library.h"

#include <stdio.h>
#include <stdlib.h>

char **makeArray(int n)
{
    return calloc(n,sizeof(char*));
}
void deleteArray (char** array, int n) {
    for (int i = 0; i < n; i++) {
        free(array[i]);
    }
    free(array);
}
char *addBlock(char **array, int n, size_t size) {
    array[n] =  calloc(size, sizeof(char));
    return array[n];
}
void removeBlock(char **array, int i) {
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

char *findClosestBlock(char **array, int n, int blockindex) {
    int blocktocompare = sizeOfBlock(array[blockindex]); // wart. ascii bloku z ktorym porownojemy inne
    char* closestblock;   // blok najblizszy rozmiarem
    int diffbetweenblocks; // roznica kodow ascii porownywanego bloku i najblizszego rozmiarem bloku
    if (blockindex!=0) { // wtedy pierwszym porownywanym blokiem bd array[0]
        diffbetweenblocks = abs(sizeOfBlock(array[0]) - blocktocompare) ;
        closestblock = array[0];
    }
    else { // w przeciwnym wypadku pierwszym porownywanym blokiem bedzie array[1]
        diffbetweenblocks = abs(sizeOfBlock(array[1]) - blocktocompare);
        closestblock = array[1];
        }
    for (int i = 1; i < n; i++) {
        if (i != blockindex) {
            if ( abs(sizeOfBlock(array[i]) - blocktocompare) < diffbetweenblocks) {
                diffbetweenblocks = abs(sizeOfBlock(array[i]) - blocktocompare);
                closestblock = array[i];
            }
        }
    }
    return closestblock;
}

int sizeOfStaticBlock (int n)
{
    if (n<10) {
        int size = 0;
        for (int i = 0; tab[n][i] != '\0' && i < 10; i++) {
            size += (int) tab[n][i];
        }
        return size;
    }
    return 0;
}
int findClosestBlockInStatic (int blockindex)
{
    if (blockindex<10 && blockindex>-1) {
        int blocktocompare = sizeOfStaticBlock(blockindex); // wart. ascii wiersza, z ktorym porownojemy inne
        int indexOfClosestBlock;   // index wiersza najblizszego rozmiarem
        int diffbetweenblocks; // roznica kodow ascii porownywanego wiersza i najblizszego rozmiarem wiersza
        if (blockindex != 0) { // wtedy pierwszym porownywanym wierszem bd tab[0]
            diffbetweenblocks = abs(sizeOfStaticBlock(0) - blocktocompare);
            indexOfClosestBlock = 0;
        } else { // w przeciwnym wypadku pierwszym porownywanym wierszem bedzie tab[1]
            diffbetweenblocks = abs(sizeOfStaticBlock(1) - blocktocompare);
            indexOfClosestBlock = 1;
        }
        for (int i = 1; i < 10; i++) {
            if (i != blockindex && tab[i] != '\0') {
                if (abs(sizeOfStaticBlock(i) - blocktocompare) < diffbetweenblocks) {
                    diffbetweenblocks = abs(sizeOfStaticBlock(i) - blocktocompare);
                    indexOfClosestBlock = i;
                }
            }
        }
        return indexOfClosestBlock;
    }
    return -1;
}