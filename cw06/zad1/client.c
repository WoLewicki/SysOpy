#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <sys/msg.h>
#include <bits/signum.h>
#include <signal.h>
#include <unistd.h>
#include "specifications.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define LINE_MAX 120


int clientqueue = -1;
int mainqueue = -1;

void removequeue()
{
    if (clientqueue > -1)
    {
        if (msgctl(clientqueue, IPC_RMID, NULL) < 0) printf("Couldn't remove client queue atexit.\n");
        printf("CLIENT %d: Ending program after having removed queue.\n",getpid());
    }
    if (mainqueue > -1)
    {	
    	Msg msg;
    	msg.mtype = STOP;
        msg.pid = getpid();
        if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) printf("Couldn't send stop msg to server in process %d. Prolly server already stopped working.\n", getpid());
        else printf("CLIENT %d: Sent STOP msg to server. Should remove my queue from array.\n", getpid());
    }
}

void inthandler(int signo)
{
    if (signo == SIGINT) exit(1);
}


int main(int argc, char *argv[]) {
	if (argc < 2) FAILURE_EXIT(1, "Pass at least 1 arg.\n");
    if (atexit(removequeue) < 0) FAILURE_EXIT(1, "Couldn't register atexit function.\n");
    signal (SIGINT, inthandler);
    FILE *input;
    int switcher = strtol(argv[1], NULL, 10);
    if (switcher == 1)
    {

        input = fopen(argv[2], "r");
        if (input == NULL) FAILURE_EXIT(1, "Couldn't find file to open.\n");
    }

    char* path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(1, "Couldn't get HOME envvar.\n");

    key_t mainkey = ftok(path, ID);
    mainqueue = msgget(mainkey, 0); // ta kolejka juz powinna istniec bo tworzy ja serwer
    if (mainqueue < 0) FAILURE_EXIT(1, "Coulnd't get server's queue. Propably some other process sent END message before opening in this one.\n");

    key_t clientkey = ftok(path, getpid());
    clientqueue = msgget(clientkey, IPC_CREAT | 0666);
    if (clientqueue < 0) FAILURE_EXIT(1, "Coulnd't make client's queue of pid: %d\n", getpid());
    Msg msg;
    msg.mtype = START;
    msg.pid = getpid();
    sprintf(msg.mtext, "%d", clientkey);
    if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1, "Couldn't send start request to server in pid: %d\n", getpid());
    if (msgrcv(clientqueue, &msg, MAXMSG, 0 ,0) < 0) FAILURE_EXIT(1, "Couldn't receive start permission request to server in pid: %d\n", getpid());
    int clientID = (int) strtol(msg.mtext, NULL, 10);
    if (clientID == -1) FAILURE_EXIT(1, "No place for client of pid: %d\n", getpid());
    printf("Process of pid: %d has ID : %d\n", getpid(), clientID);

    if (switcher == 1) {
        char line[LINE_MAX];
        while (fgets(line, LINE_MAX, input)) { // pierwszy argument w linii to nr operacji, potem odpowiednie wartosci
            char *word = line;
            if (strcmp(word, "\n") == 0) continue;
            char *firstarg = strtok_r(NULL, " \n\t", &word);
            if (word == NULL) continue;
            switch ((int) strtol(firstarg, NULL, 10)) {
                case 2: // mirror
                    msg.mtype = MIRROR;
                    if (word == NULL) FAILURE_EXIT(1, "Passed blank line after mirror function.\n");
                    sprintf(msg.mtext, "%s", word); // powinno przypisac wszystko po argumencie
                    msg.pid = getpid();
                    if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                             "Couldn't send mirror msg to server in process %d\n",
                                                                             getpid());
                    if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1,
                                                                                  "Couldn't receive mirror msg from server in process %d\n",
                                                                                  getpid());
                    printf("%s\n", msg.mtext);
                    break;
                case 3: // calc
                    msg.mtype = CALC;
                    if (word == NULL) FAILURE_EXIT(1, "Passed blank line after calc function.\n");
                    sprintf(msg.mtext, "%s", word); // powinno przypisac dzialanie np ADD 3 2
                    msg.pid = getpid();
                    if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                             "Couldn't send calc msg to server in process %d\n",
                                                                             getpid());
                    if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1,
                                                                                  "Couldn't receive calc msg from server in process %d\n",
                                                                                  getpid());
                    if (strtol(msg.mtext, NULL, 10) == -1)
                        printf("Couldn't calculate expression. Prolly tried to div by 0. Continueing.\n");
                    else printf("%s\n", msg.mtext);
                    break;
                case 4: //time
                    msg.mtype = TIME;
                    msg.pid = getpid();
                    if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                             "Couldn't send time msg to server in process %d\n",
                                                                             getpid());
                    if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1,
                                                                                  "Couldn't receive time msg from server in process %d\n",
                                                                                  getpid());
                    printf("%s\n", msg.mtext);
                    break;
                case 5: //end
                    msg.mtype = END;
                    if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                             "Couldn't send end msg to server in process %d\n",
                                                                             getpid());
                    fclose(input);
                    return 0;
                default:
                    printf("Got other command. Continueing\n");
                    continue;
            }
        }
    }
    else if (switcher == 2)
    {
        char cmd[20];
        while(1)
        {
            msg.pid = getpid();
            printf("Enter your request: \n");
            if(fgets(cmd, 20, stdin) == NULL){
                printf("Error reading your command!\n");
                continue;
            }
            int n = strlen(cmd);
            if(cmd[n-1] == '\n') cmd[n-1] = 0;

            if(strcmp(cmd, "mirror") == 0) {
                msg.mtype = MIRROR;
                printf("Enter string of characters to Mirror: ");
                if (fgets(msg.mtext, MAXMSG, stdin) == NULL) 
                {
                	printf("Too many characters!\n");
                	continue;
                }	

                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                         "Couldn't send mirror msg to server in process %d\n",
                                                                         getpid());
                if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1,
                                                                              "Couldn't receive mirror msg from server in process %d\n",
                                                                              getpid());
                printf("%s\n", msg.mtext);
            }else if(strcmp(cmd, "calc") == 0){
                msg.mtype = CALC;
                printf("Enter expression.\n");
                if (fgets(msg.mtext, MAXMSG, stdin) == NULL)
                {
                 printf("Too many characters!\n");
                 continue;
                } 
                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                         "Couldn't send calc msg to server in process %d\n",
                                                                         getpid());
                if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1,
                                                                              "Couldn't receive calc msg from server in process %d\n",
                                                                              getpid());
                if (strtol(msg.mtext, NULL, 10) == -1)
                    printf("Couldn't calculate expression. Prolly tried to div by 0. Continueing.\n");
                else printf("%s\n", msg.mtext);

            }else if(strcmp(cmd, "time") == 0){
                msg.mtype = TIME;
                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                         "Couldn't send time msg to server in process %d\n",
                                                                         getpid());
                if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1,
                                                                              "Couldn't receive time msg from server in process %d\n",
                                                                              getpid());
                printf("%s\n", msg.mtext);
            }else if(strcmp(cmd, "end") == 0){
                msg.mtype = END;
                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1,
                                                                         "Couldn't send end msg to server in process %d\n",
                                                                         getpid());
                return 0;
            }
            else printf("Wrong command!\n");
        }
    }
    return 0;
}//
// Created by wojlewy on 20.04.18.
//

