#ifndef UNTITLED_LIBRARY_H
#define UNTITLED_LIBRARY_H
#include <stdio.h>
#include <stdlib.h>
char tab[10][10];
char ** makeArray(int); // przekaz: ilosc blokow
void deleteArray (char** array, int n); // przekaz: tablice wskaznikow, dlugosc tablicy wskaznikow
char *addBlock(char **array, int n, size_t size); // przekaz: tablice wskaznikow, index tab. wsk., dl. bloku
void removeBlock(char **array, int i); // przekaz: tab. wsk., index tab. wsk. do usuniecia
char *findClosestBlock(char **array, int n, int blockindex); // przekaz: tab. wsk. , dl. tab. wsk., blok z ktorym mamy porownywac
int sizeOfBlock(char *block); // przekaz: blok
char *makeStaticArray (); // przekazuje tablice [10][10]
int sizeOfStaticBlock (int); // przekaz: nr wiersza tablicy
int findClosestBlockInStatic (int blockindex); // przekaz: index bloku z ktorym mamy porownywac
#endif
