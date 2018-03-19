//
// Created by wojlewy on 13.03.18.
//

#ifndef ZAD3B_LIBRARY_H
#define ZAD3B_LIBRARY_H

#include <stdio.h>
#include <stdlib.h>

char tab[100][100];
char **allocarray (long arsize, long blocksize, long alloctype, long createarray, long addnumber);
void addblocks(char **array, long addnumber, long blocksize, long arsize);
void deleteblocks(char **array, long deletenumber, long arsize, long blocks);
void calctimediff (struct timeval tv1,struct timeval tv2);
void preparetab();




char ** makeArray(long); // przekaz: ilosc blokow
void deleteArray (char** array, long n, long blocks); // przekaz: tablice wskaznikow, dlugosc tablicy wskaznikow, liczbe zaalokowanych blokow
char *addBlock(char **array, long n, size_t size); // przekaz: tablice wskaznikow, index tab. wsk., dl. bloku
void removeBlock(char **array, long i); // przekaz: tab. wsk., index tab. wsk. do usuniecia
char *findClosestBlock(char **array, long n, long blockindex); // przekaz: tab. wsk. , dl. tab. wsk., blok z ktorym mamy porownywac
int sizeOfBlock(char *block); // przekaz: blok
int sizeOfStaticBlock (long); // przekaz: nr wiersza tablicy
int findClosestBlockInStatic (long blockindex); // przekaz: index bloku z ktorym mamy porownywac
#endif //ZAD3B_LIBRARY_H
