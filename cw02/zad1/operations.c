#include "operations.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>

void generate (char *filename, size_t records, size_t length)
{
        FILE *source = fopen(filename, "w+");
        if (source) {
            for (int j = 0; j < records; ++j) {

                char *string = malloc(length);
                for (int i = 0; i < length; ++i) {
                    string[i] = (char) ((rand() % 94) + 32);
                }
                fwrite(string, 1, length, source);
                free(string);
            }
        }
        fclose(source);
}
void libsort (char *filename, size_t records, size_t length)
{
    int helper;
    u_char swapkey[1];
    u_char key[1];
    char *tmptab = malloc(length);//pomocnicza tablica do przepisania wartosci
    char *keyholder = malloc(length);//pomocnicza tablica do trzymania calego rekordu klucza
    FILE *source = fopen(filename,"r+");
    if (source)
    {
        for (int i = 1; i <records ; i++)
        {
            fseek(source, i*length,0 ); //ustawiam wskaznik na i-ty "element"
            fread(keyholder, 1, length,source); // kopiuje rekord elementu do keyholdera
            fseek(source, (-length),1); // wracam na poprzednie miejsce
            fread(key, 1, 1, source); // pobieram wartosc pierwszego znaku
            fseek(source, (-length-1), 1); // cofam sie o jeden "element" i 1 do tylu bo fread przesunelo wskaznik
            fread(swapkey, 1, 1, source); // pobieram wartosc
            fseek(source, -1, 1); // wracam po pobraniu wartosci, jestem na j elemencie
            helper =i-1; // flaga do kontrolowania poczatku pliku
            while(helper>=0 && swapkey[0]>key[0]) //sprawdza czy jest cos za mna i wartosci charow
            {
                fread(tmptab, 1, length, source); // wpisz znaki do pomocniczej tablicy, jestesmy do przodu o length
                fwrite(tmptab, 1, length, source); //przepisz element do elementu o jeden wiekszego
                if (helper>0) fseek(source, 3 *(-length), 1); //bylem 2 elementy dalej niz gdy wszedlem do while, musze sie cofnac wiec o 3, ale tylko jesli nie bd to przed poczatkiem pliku
                else fseek(source, 2*(-length),1); // gdy bylem na poczatku petli while na poczatku pliku, musze do niego wrocic
                fread(swapkey, 1, 1, source); // ustawiam nowy swapkey do porownywania
                fseek(source, -1, 1); // wracam po przeczytaniu
                helper--; //zmniejszam flage do kontroli poczatku pliku
            }
            if (helper==-1) fwrite(keyholder, 1, length, source); // jestem na poczatku pliku i tam wpisuje wartosc
            else
            {
                fseek(source, length,1); // bylismy na elemencie mniejszym, a mamy wpisac cos po nim, wiec musimy sie przesunac o 1 element
                fwrite(keyholder, 1, length, source);
            }
        }
        fclose(source);
    }
    free(tmptab);
    free(keyholder);
}
void syssort (char *filename, size_t records, size_t length) // funkcja tworzona na podstawie libsort
{
    int helper;
    int source= open(filename, O_RDWR);
    if(source != -1)
    {
        u_char swapkey[1];
        u_char key[1];
        char *tmptab = malloc(length);
        char *keyholder = malloc(length);
        for (int i = 1; i < records; i++) {
            lseek(source, i * length, SEEK_SET);
            read(source, keyholder, length);
            lseek(source, (-length), SEEK_CUR);
            read(source, key, 1);
            lseek(source, (-length - 1), SEEK_CUR);
            read(source, swapkey, 1);
            lseek(source, -1, SEEK_CUR);
            helper = i - 1;
            while (helper >= 0 && swapkey[0] > key[0]) {
                read(source, tmptab, length);
                write(source, tmptab, length);
                if (helper > 0) lseek(source, 3 * (-length), SEEK_CUR);
                else lseek(source, 2 * (-length), SEEK_CUR);
                read(source, swapkey, 1);
                lseek(source, -1, SEEK_CUR);
                helper--;
            }
            if (helper == -1) write(source, keyholder, length);
            else {
                lseek(source, length, SEEK_CUR);
                write(source, keyholder, length);
            }
        }
        free(tmptab);
        free(keyholder);
        close(source);
    }
}
void libcopy (char *fromfile, char *tofile, size_t records, size_t buffersize)
{
    FILE *from = fopen(fromfile, "r");
    FILE *to = fopen(tofile, "w+");
    if(from && to)
    {
        ssize_t bufferbytes; // musimy zabezpieczyc sie przed koncem pliku przy wpisywaniu
        char *buffer = malloc(buffersize);
        int i =0;
        while ((bufferbytes = fread(buffer, 1, buffersize, from))>0 && i < records) // dopoki jest co kopiowac to bedzie sie to ladowac do bufora
        {
            fwrite(buffer, 1, (size_t) bufferbytes, to); // wartosc bufora wpisujemy do pliku
            i++;
        }
        free(buffer);
    }
    fclose(from);
    fclose(to);
}
void syscopy (char *fromfile, char *tofile, size_t records, size_t buffersize) // funkcja tworzona na podstawie libcopy
{
    int from = open(fromfile, O_RDONLY);
    int trunc = open(tofile, O_TRUNC);
    close(trunc);
    int to = open(tofile, O_WRONLY);
    if (from != -1 && to != -1)
    {
        ssize_t bufferbytes;
        char *buffer = malloc(buffersize);
        int i =0;
        while ((bufferbytes=read(from, buffer, sizeof(buffersize)))> 0 && i< records)
        {
            write(to, buffer,(size_t) bufferbytes);
            i++;
        }
    }
    close(from);
    close(to);
}
//
// Created by wojlewy on 14.03.18.
//

