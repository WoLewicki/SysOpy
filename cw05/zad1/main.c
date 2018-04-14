#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#define ARGS_MAX 12
#define LINE_MAX 120
#define PIPES_MAX 10

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}


int main(int argc, char *argv[]) {
	if (argc != 2)
	{
		printf("Pass file name.\n");
		exit(1);
	}
	char *filename = argv[1];
    FILE *input = fopen(filename, "r");
    if (input == NULL)
    {
        exit(EXIT_FAILURE);
    }
    int len = LINE_MAX;
    char line[ARGS_MAX * PIPES_MAX];
    int fds[2][2];
    int proccounter =0;
    while (fgets(line, len, input))
    {
        char *word = line;
        int i = 0;
        int j = 0;
        int lastpipeindex =0;
        int pipescounter =0;
        char** *procargs = calloc(PIPES_MAX, sizeof(char **)); // tablica wskaznikow na tablice argumentow execa
        char** pipes=  calloc(PIPES_MAX*ARGS_MAX, sizeof(char*)); // przyjmuje do ARGS_MAX argumentow
        if (strcmp(line, "\n") == 0) FAILURE_EXIT(1, "Blank line detected. Ending.\n");
        while ((pipes[i++] = strtok_r(NULL, " \n", &word)) && i<ARGS_MAX*PIPES_MAX); // rozdzielenie linijki na pojedyncze wyrazy
        printf("Executing line: ");
        for (int l =0; l< i -1; l++)
        {
        	printf("%s ",pipes[l]);
        }
        printf("\n");
        while (j < i) {
            char **arguments = calloc(ARGS_MAX, sizeof(char *));
            while (j < i && pipes[j] != NULL && strcmp(pipes[j], "|") != 0) {
                arguments[j - lastpipeindex] = pipes[j];
                j++;
            } // po wyjsciu j wskazuje na pipe
            if (j < i) j++; // j wskazuje na pierwszy element po pipe, jesli doszlismy do konca linii to nie przesuwamy bo nie ma juz pipe
            lastpipeindex = j; // lastpipeindex teraz tez wskazuje na element po pipe
            procargs[pipescounter] = arguments;
            pipescounter++;
        } // dzieki temu posiadam tablice z argumentami wywolan execa dla kazdego forka
        if (pipescounter == 0) FAILURE_EXIT(1, "No processes to make, 0 commands passed.\n");
        int lastinline =0;
       for (proccounter = 0; proccounter < pipescounter; proccounter++)// dopoki mam kolejne listy argumentow do wywolania execa
       {
           if (proccounter > 1) {
               close(fds[proccounter % 2][STDIN_FILENO]);
               close(fds[proccounter % 2][STDOUT_FILENO]);
           }
           if (pipe(fds[proccounter % 2]))
               printf("Couldn't pipe in  %d process.\n", proccounter);

           pid_t process = fork();
           if (process == -1) FAILURE_EXIT(1, "Couldnt fork %d process.\n", proccounter);
           if (process == 0) {
               if (proccounter < pipescounter- 1) { // ostatni proces nie zmienia STDOUT
                   close(fds[proccounter % 2][STDIN_FILENO]);
                   if (dup2(fds[proccounter % 2][STDOUT_FILENO], 1) < 0) FAILURE_EXIT(1, "Couldnt set writing in %d process.\n", proccounter);
               }
               if (proccounter != 0) {
                   close(fds[(proccounter + 1) % 2][STDOUT_FILENO]); // pierwszy nie czyta z fds
                   if (dup2(fds[(proccounter + 1) % 2][STDIN_FILENO], 0) < 0) FAILURE_EXIT(1, "Couldnt set reading in %d process.\n", proccounter);
               }
               if (execvp(procargs[proccounter][0], procargs[proccounter]) <0 )
               FAILURE_EXIT(1, "Failed to execute %d process.\n", proccounter);
                        }
           lastinline = process;
       }
        for (int k = 0; k <pipescounter ; ++k) free(procargs[k]);
        free(procargs);
        free(pipes);

        close(fds[proccounter % 2][STDIN_FILENO]);
        close(fds[proccounter % 2][STDOUT_FILENO]);
        waitpid(lastinline, NULL, 0);
    }
    while(wait(NULL)) // zwraca -1 gdy nie ma juz dzieci wiec poczeka na zakonczenie wszystkich procesow
    {
        if (errno == ECHILD) {
            break;
        }
    }
    fclose(input);
    return 0;
}
