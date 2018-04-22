#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#include "specifications.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

void startingtask(struct Msg *);
void mirrortask (struct Msg *);
void calctask (struct Msg *);
void timetask (struct Msg *);
int prepmsg(struct Msg *);
int findclientqueue(pid_t);



int clientsarray[MAXCLIENTS][2];
int clientscounter =0;
int waiting =1;
int mainqueue = -1;

void removequeue()
{
    if (mainqueue > -1)
    {
        for (int i=0;i<clientscounter; i++) mq_close(clientsarray[i][1]);
        if (mq_close(mainqueue)< 0) printf("SERVER: Couldn't close main queue atexit.\n");
        if (mq_unlink(serverpath) < 0 ) printf("SERVER: Couldn't unlink main queue.\n");
        else printf("SERVER: Ending program after having removed queue.\n");
    }
}

void inthandler(int signo)
{
    if (signo == SIGINT) exit(1);
}

int main() {
    if (atexit(removequeue) < 0) FAILURE_EXIT(1, "Couldn't register atexit function.\n");
    signal (SIGINT, inthandler);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MSGSLIMIT;
    attr.mq_msgsize = MAXMSG;
    attr.mq_curmsgs = 0;

    if ((mainqueue = mq_open(serverpath, O_CREAT | O_RDONLY, 0666, &attr)) < 0) FAILURE_EXIT(1, "SERVER: Couldn't make server's queue.\n");
    struct mq_attr mesqueue;
    Msg receiver;
    while (1) // odbieranie komunikatow i wykonywanie polecen
    {
        if (waiting == 0) {
            if (mq_getattr(mainqueue, &mesqueue) != 0) FAILURE_EXIT(1, "Couldn't get current main queue state.\n");
            if (mesqueue.mq_curmsgs == 0)
            {
                printf("SERVER: Ending main queue.\n");
                break;
            }
        }
        if (mq_receive(mainqueue, (char *)&receiver, MAXMSG, NULL) < 0) FAILURE_EXIT(1, "Couldn't receive any message from not empty queue.\n");
        switch (receiver.mtype)
        {
            case START:
                startingtask(&receiver);
                break;
            case MIRROR:
                mirrortask(&receiver);
                break;
            case CALC:
                calctask(&receiver);
                break;
            case TIME:
                timetask(&receiver);
                break;
            case END:
                waiting =0;
                printf("SERVER: Got END message. Will process all received messages and then will terminate.\n");
                break;
            case CLOSE:
                if (mq_close(clientsarray[receiver.pid][1]) < 0) printf("CLIENT: Couldn't close client queue atexit.\n");
                break;
            default:
            FAILURE_EXIT(1, "Wrong task type passed.\n");
        }
    }
    return 0;
}

void startingtask (struct Msg *msger) {
    char clientpath[20];
    int clientpid = msger->pid;
    sprintf(clientpath, "/%d", clientpid);
    int clientqueue;
    if ((clientqueue = mq_open(clientpath, O_WRONLY)) < 0) FAILURE_EXIT(1, "Couldn't open client's queue.\n");
    printf("%d eh\n", clientqueue);
    msger->mtype = START;
    msger->pid = getpid();

    if (clientscounter >= MAXCLIENTS) {
        printf("Too many clients.\n");
        sprintf(msger->mtext, "%d", -1);
        if (mq_send(clientqueue, (char *) msger, MAXMSG, 1) == -1) FAILURE_EXIT(1, "Couldn't send info to client.\n");
        if (mq_close(clientqueue) <0) FAILURE_EXIT(1, "Couldn't close client's queue.\n");
    } else {
        printf("Doszedlem tutaj.\n");
        clientsarray[clientscounter][0] = clientpid;
        clientsarray[clientscounter][1] = clientqueue;
        sprintf(msger->mtext, "%d", clientscounter); // przydzielam klientowi jego ID na podstawie tego kiedy przyslal prosbe
        clientscounter++;
        if (mq_send(clientqueue, (char *) msger, MAXMSG, 1) == -1) FAILURE_EXIT(1, "Couldn't send access to queue to client.\n");
    }
    printf("Wyslalem.\n");
}

void mirrortask (struct Msg *msger)
{
    int clientqueue = prepmsg(msger);
    if (clientqueue == -1) FAILURE_EXIT(1, "Couldn't find client's ID\n");
    char *msg = msger->mtext;
    if (strlen(msg) == 0) FAILURE_EXIT(1, "The string was empty in mirror task.\n");
    char temp;
    size_t len = strlen(msg) -1;
    size_t k = len;
    for (size_t i=0;i< len/2; i++, k--)
    {
        temp = msg[i];
        msg[i] = msg[k];
        msg[k] = temp;
    }
    sprintf(msger->mtext, "%s", msg);
    if (mq_send(clientqueue, (char *)msger, MAXMSG, 2) == -1) FAILURE_EXIT(1, "Couldn't send result of mirrortask to client.\n");
}
void calctask (struct Msg *msger)
{
    int clientqueue = prepmsg(msger);
    if (clientqueue == -1) FAILURE_EXIT(1, "Couldn't find client's ID\n");
    char *msg = msger->mtext;
    char *helper =msg;
    char *task;
    char *firstarg;
    char*secondarg;
    int result =0;
    if (strcmp(msg, "\n") == 0) FAILURE_EXIT(1, "Passed blank line to calctask.\n");
    task = strtok_r(NULL, " \n\t", &helper);
    firstarg = strtok_r(NULL, " \n\t", &helper);
    secondarg = strtok_r(NULL, " \n\t", &helper);
    if (strcmp(task, "ADD") == 0) result = (int) (strtol(firstarg,NULL, 10) + strtol(secondarg,NULL, 10));
    else if (strcmp(task, "MUL") == 0) result = (int) (strtol(firstarg,NULL, 10) * strtol(secondarg,NULL, 10));
    else if (strcmp(task, "SUB") == 0) result = (int) (strtol(firstarg,NULL, 10) - strtol(secondarg,NULL, 10));
    else if (strcmp(task, "DIV") == 0 && strtol(secondarg, NULL, 10) != 0) result = (int) (strtol(firstarg,NULL, 10) / strtol(secondarg,NULL, 10));
    else FAILURE_EXIT(1, "Wrong task name or tried to div by 0.\n");
    sprintf(msger->mtext, "%d", result);
    if (mq_send(clientqueue, (char *)msger, MAXMSG, 2) == -1) FAILURE_EXIT(1, "Couldn't send result of calctask to client.\n");
}

void timetask (struct Msg *msger)
{
    int clientqueue = prepmsg(msger);
    if (clientqueue == -1) FAILURE_EXIT(1, "Couldn't find client's ID\n");
    char buff[20];
    time_t now = time(NULL);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    sprintf(msger->mtext, "%s", buff);
    if (mq_send(clientqueue, (char *)msger, MAXMSG, 2) == -1) FAILURE_EXIT(1, "Couldn't send result of timetask to client.\n");
}

int prepmsg(struct Msg *msger)
{
    int clientqueue = findclientqueue(msger->pid);
    msger->mtype = msger->pid;
    msger->pid = getpid();
    return clientqueue;
}

int findclientqueue(pid_t pid)
{
    for (int i =0; i<MAXCLIENTS; i++)
    {
        if (clientsarray[i][0] == pid) return clientsarray[i][1];
    }
    return -1;
}