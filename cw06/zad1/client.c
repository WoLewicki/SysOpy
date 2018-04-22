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

void removequeue()
{
    if (clientqueue > -1)
    {
        if (msgctl(clientqueue, IPC_RMID, NULL) < 0) printf("Couldn't remove main queue atexit.\n");
        printf("CLIENT %d: Ending program after having removed queue.\n",getpid());
    }
}

void inthandler(int signo)
{
    if (signo == SIGINT) exit(1);
}


int main(int argc, char *argv[]) {
    sleep(1);
    if (atexit(removequeue) < 0) FAILURE_EXIT(1, "Couldn't register atexit function.\n");
    signal (SIGINT, inthandler);

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) FAILURE_EXIT(1, "Couldn't find file to open.\n");


    char* path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(1, "Couldn't get HOME envvar.\n");

    key_t mainkey = ftok(path, ID);
    int mainqueue = msgget(mainkey, 0); // ta kolejka juz powinna istniec bo tworzy ja serwer
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

    char line[LINE_MAX];
    while (fgets(line, LINE_MAX, input)) { // pierwszy argument w pliku to nr operacji, potem odpowiednie wartosci
        char *word = line;
        char *firstarg = strtok_r(NULL, " \n\t", &word);
        switch ((int) strtol(firstarg, NULL, 10))
        {
            case 2: // mirror
                msg.mtype = MIRROR;
                if (word == NULL) FAILURE_EXIT(1, "Passed blank line after mirror function.\n");
                sprintf(msg.mtext, "%s", word); // powinno przypisac wszystko po argumencie
                msg.pid = getpid();
                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1, "Couldn't send mirror msg to server in process %d\n", getpid());
                if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1, "Couldn't receive mirror msg from server in process %d\n", getpid());
                printf("%s\n", msg.mtext);
                break;
            case 3: // calc
                msg.mtype = CALC;
                if (word == NULL) FAILURE_EXIT(1, "Passed blank line after calc function.\n");
                sprintf(msg.mtext, "%s", word); // powinno przypisac dzialanie np ADD 3 2
                msg.pid = getpid();
                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1, "Couldn't send calc msg to server in process %d\n", getpid());
                if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1, "Couldn't receive calc msg from server in process %d\n", getpid());
                printf("%s\n", msg.mtext);
                break;
            case 4: //time
                msg.mtype = TIME;
                msg.pid = getpid();
                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1, "Couldn't send time msg to server in process %d\n", getpid());
                if (msgrcv(clientqueue, &msg, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1, "Couldn't receive time msg from server in process %d\n", getpid());
                printf("%s\n", msg.mtext);
                break;
            case 5: //end
                msg.mtype = END;
                if (msgsnd(mainqueue, &msg, MAXMSG, 0) < 0) FAILURE_EXIT(1, "Couldn't send end msg to server in process %d\n", getpid());
                break;
            default:
                printf("Got other command so exiting.\n");
                exit(0);
        }
    }
    fclose(input);
    return 0;
}//
// Created by wojlewy on 20.04.18.
//

