#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    for (int i=1;i<argc;i++)
    {
    printf("Argument nr%d o wartosci: %d\n", i, (int) strtol (argv[i], NULL, 10));
    }
    return 0;
}
