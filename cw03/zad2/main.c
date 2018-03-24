#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <wait.h>
#define ARGS_MAX 12
#define LINE_MAX 120

int main(int argc, char *argv[]) {
	if (argc != 2)
	{
		printf("Pass file name.");
		exit(1);
	}
	char *filename = argv[1];
    FILE *input = fopen(filename, "r");
    if (input == NULL)
    {
        exit(EXIT_FAILURE);
    }
    size_t len = 0;
    char *line = calloc(LINE_MAX, sizeof(char));
    while (getline(&line, &len, input) != -1) {
        char *word;
        word = strtok (line," \n");
        int i = 0;
        char **arguments = calloc(ARGS_MAX, sizeof(char*)); // przyjmuje do ARGS_MAX argumentow
        while (line !=NULL && word !=NULL && i<ARGS_MAX)
        {
            arguments[i] = word;
            word = strtok (NULL, " \n\t");
            i++;
        }
        int processresult;
        pid_t process = vfork();
        if (process == 0)
        {
            if (strcmp(arguments[0], "0") == 0)
            {
                printf("Didn't pass function name.\n");
                exit(1);
            }
            printf("Executing in process of pid : %d with arguments : ",getpid());
            for (int j = 0; j <i ; ++j) {
                printf("%s ",arguments[j]);
            }
            printf("\n");
            execvp(arguments[0], arguments);
        }
        else{
        if(process != wait(&processresult)) {
            printf("Something went wrong.\n");
            exit(1);
        }
        if (processresult != 0)
        {
            printf("Something gone wrong in command : %s\n", arguments[0]);
            exit(1);
        }
        free(arguments);
        }
    }
    fclose(input);
    return 0;
}
