//
// Created by wojlewy on 11.03.18.
//

#ifndef ZAD2_LIBRARY_H
#define ZAD2_LIBRARY_H
#include <stdio.h>
#include <stdlib.h>

char tab[100][100];
char ** makeArray(long); // przekaz: ilosc blokow
void deleteArray (char** array, long n, long blocks); // przekaz: tablice wskaznikow, dlugosc tablicy wskaznikow, liczbe zaalokowanych blokow
char *addBlock(char **array, long n, size_t size); // przekaz: tablice wskaznikow, index tab. wsk., dl. bloku
void removeBlock(char **array, long i); // przekaz: tab. wsk., index tab. wsk. do usuniecia
char *findClosestBlock(char **array, long n, long blockindex); // przekaz: tab. wsk. , dl. tab. wsk., blok z ktorym mamy porownywac
int sizeOfBlock(char *block); // przekaz: blok
int sizeOfStaticBlock (long); // przekaz: nr wiersza tablicy
int findClosestBlockInStatic (long blockindex); // przekaz: index bloku z ktorym mamy porownywac
#endif