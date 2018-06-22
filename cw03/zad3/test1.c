#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
	int i,j;
	int tablica[10000];
    while(1)
    {
    for (i=0; i<10000; i++) tablica[i]=1;
	for (i=1; i<100; i++)
		{
			if (tablica[i] != 0)
				{
					j = i+i;
					while (j<100)
						{
						tablica[j] = 0;
						j += i;
						}
				}
		}
    }
    return 0;
}
