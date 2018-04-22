#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>

#include "specifications.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

#define LINE_MAX 120

int clientID = -1;
int clientqueue = -1;
int mainqueue = -1;
char path[20];

void removequeue()
{
    if (mainqueue > -1 || clientqueue > -1)
    {
        if (mq_close(mainqueue)< 0) printf("CLIENT: Couldn't close main queue atexit.\n");
        if (mq_close(clientqueue) < 0) printf("CLIENT: Couldn't close client queue atexit.\n");
        if (mq_unlink(path) < 0) printf("Couldn't remove client queue atexit.\n");
        printf("CLIENT: Ending program after having removed queue.\n");
    }
}

void inthandler(int signo)
{
    if (signo == SIGINT) exit(1);
}


int main(int argc, char *argv[]) {
    if (argc != 2) FAILURE_EXIT(1, "Pass input.txt.\n");
    FILE *input = fopen(argv[1], "r");
    if (atexit(removequeue) < 0) FAILURE_EXIT(1, "Couldn't register atexit function.\n");
    if(signal(SIGINT, inthandler) == SIG_ERR) FAILURE_EXIT(1, "Registering INT failed!");
    sprintf(path, "/%d", getpid());

    mainqueue = mq_open(serverpath, O_WRONLY);
    if(mainqueue == -1) FAILURE_EXIT(3, "Opening public queue failed!");

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MSGSLIMIT;
    posixAttr.mq_msgsize = MAXMSG;

    clientqueue = mq_open(path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(clientqueue == -1) FAILURE_EXIT(3, "Creation of private queue failed!");

    Msg msg;

    msg.mtype = START;
    msg.pid = getpid();
    if(mq_send(mainqueue, (char*) &msg, MAXMSG, 1) == -1) FAILURE_EXIT(3, "Login request failed!");
    if(mq_receive(clientqueue,(char*) &msg, MAXMSG, NULL) == -1) FAILURE_EXIT(1, "Catching START response failed!");
    int clientID = (int) strtol(msg.mtext, NULL, 10);
    if(clientID < 0) FAILURE_EXIT(1, "Server cannot have more clients!");
    printf("Client registered! My session nr is %d!\n", clientID);

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
                if (mq_send(mainqueue, (char *)&msg, MAXMSG, 2) < 0) FAILURE_EXIT(1, "Couldn't send mirror msg to server in process %d\n", getpid());
                if (mq_receive(clientqueue, (char *)&msg, MAXMSG, NULL) < 0) FAILURE_EXIT(1, "Couldn't receive mirror msg from server in process %d\n", getpid());
                printf("%s\n", msg.mtext);
                break;
            case 3: // calc
                msg.mtype = CALC;
                if (word == NULL) FAILURE_EXIT(1, "Passed blank line after calc function.\n");
                sprintf(msg.mtext, "%s", word); // powinno przypisac dzialanie np ADD 3 2
                msg.pid = getpid();
                if (mq_send(mainqueue, (char *)&msg, MAXMSG, 2) < 0) FAILURE_EXIT(1, "Couldn't send calc msg to server in process %d\n", getpid());
                if (mq_receive(clientqueue, (char *)&msg, MAXMSG, NULL) < 0) FAILURE_EXIT(1, "Couldn't receive calc msg from server in process %d\n", getpid());
                printf("%s\n", msg.mtext);
                break;
            case 4: //time
                msg.mtype = TIME;
                msg.pid = getpid();
                if (mq_send(mainqueue, (char *)&msg, MAXMSG, 2) < 0) FAILURE_EXIT(1, "Couldn't send time msg to server in process %d\n", getpid());
                if (mq_receive(clientqueue, (char *)&msg, MAXMSG, NULL) < 0) FAILURE_EXIT(1, "Couldn't receive time msg from server in process %d\n", getpid());
                printf("%s\n", msg.mtext);
                break;
            case 5: //end
                msg.mtype = END;
                msg.pid = getpid();
                if (mq_send(mainqueue, (char *)&msg, MAXMSG, 2) < 0) FAILURE_EXIT(1, "Couldn't send end msg to server in process %d\n", getpid());
                break;
            case 6: //close server
                msg.mtype = CLOSE;
                msg.pid = getpid();
                if (mq_send(mainqueue, (char *)&msg, MAXMSG, 2) < 0) FAILURE_EXIT(1, "Couldn't send close msg to server in process %d\n", getpid());
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
