//
// Created by wojlewy on 14.03.18.
//

#ifndef ZAD1_OPERATIONS_H
#define ZAD1_OPERATIONS_H
#include <stdio.h>
#include <stdlib.h>

void generate(char *filename, size_t records, size_t length); // podaj nazwe istniejacego pliku, liczbe rekordow i ich dlugosc
void libsort (char *filename, size_t records, size_t length); // podaj nazwe istniejacego pliku, liczbe rekordow i ich dlugosc
void syssort (char *filename, size_t records, size_t length); // podaj nazwe istniejacego pliku, liczbe rekordow i ich dlugosc
void libcopy (char *fromfile, char *tofile, size_t records, size_t buffersize); // podaj skad kopiujesz, dokad kopiujesz, wielkosc bufora
void syscopy (char *fromfile, char *tofile, size_t records, size_t buffersize); // podaj skad kopiujesz, dokad kopiujesz, wielkosc bufora
#endif //ZAD1_OPERATIONS_H
