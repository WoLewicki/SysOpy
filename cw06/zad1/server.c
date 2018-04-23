#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "specifications.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

void startingtask(struct Msg *);
void mirrortask (struct Msg *);
void calctask (struct Msg *);
void timetask (struct Msg *);
void removeclienttask (struct Msg *);
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
        if (msgctl(mainqueue, IPC_RMID, NULL) < 0) printf("Couldn't remove main queue atexit.\n");
        for (int i =0; i<= clientscounter; i++) kill(clientsarray[i][0], SIGINT);
        printf("SERVER: Ending program after having removed queue.\n");
    }
}

void inthandler(int signo)
{
    if (signo == SIGINT) exit(1);
}

int main() {
    if (atexit(removequeue) < 0) FAILURE_EXIT(1, "Couldn't register atexit function.\n");
    signal (SIGINT, inthandler);
    char* path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(1, "Couldn't get HOME envvar.\n");
    key_t mainkey = ftok(path, ID);
    mainqueue = msgget(mainkey, IPC_CREAT | 0666);
    if (mainqueue < 0) FAILURE_EXIT(1, "Coulnd't make first queue.\n");

    struct msqid_ds mesqueue;
    Msg receiver;
    while (1) // odbieranie komunikatow i wykonywanie polecen
    {
        if (waiting == 0) {
            if (msgctl(mainqueue, IPC_STAT, &mesqueue) != 0) FAILURE_EXIT(1, "Couldn't get current main queue state.\n");
            if (mesqueue.msg_qnum == 0)
            {
                printf("SERVER: Ending main queue.\n");
                break;
            }
        }
        if (msgrcv(mainqueue, &receiver, MAXMSG, 0, 0) < 0) FAILURE_EXIT(1, "Couldn't receive any message from not empty queue.\n");
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
            case STOP:
            	removeclienttask(&receiver);
            	break;    
            default:
                FAILURE_EXIT(1, "Wrong task type passed.\n");
        }
    }
    return 0;
}

void startingtask (struct Msg *msger)
{
    key_t clientkey = (key_t) strtol(msger->mtext, NULL, 10);
    int clientqueue = msgget(clientkey, 0);
    int clientpid = msger->pid;
    msger->pid = getpid();
    msger->mtype = START;

    if (clientscounter >= MAXCLIENTS)
    {
        printf("Too many clients.\n");
        sprintf(msger->mtext, "%d", -1);
    }
    else
    {
        clientsarray[clientscounter][0] = clientpid;
        clientsarray[clientscounter][1] = clientqueue;
        sprintf(msger->mtext, "%d", clientscounter); // przydzielam klientowi jego ID na podstawie tego kiedy przyslal prosbe
        clientscounter++;
    }
    if (msgsnd(clientqueue, msger, MAXMSG, 0) == -1)
    {
        FAILURE_EXIT(1, "Couldn't send access to queue to client.\n");
    }
    else printf("Registered process %d with client ID %d\n", msger->pid, clientscounter-1);
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
    for (int i=0;i< len/2; i++, k--)
    {
        temp = msg[i];
        msg[i] = msg[k];
        msg[k] = temp;
    }
    sprintf(msger->mtext, "%s", msg);
    if (msgsnd(clientqueue, msger, MAXMSG, 0) == -1) FAILURE_EXIT(1, "Couldn't send result of mirrortask to client.\n");
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
    else 
    {
    sprintf(msger->mtext, "%d", -1);
    printf("Invalid operation.\n");
    if (msgsnd(clientqueue, msger, MAXMSG, 0) == -1) FAILURE_EXIT(1, "Couldn't send result of calctask to client.\n");
    return;
    }
    sprintf(msger->mtext, "%d", result);
    if (msgsnd(clientqueue, msger, MAXMSG, 0) == -1) FAILURE_EXIT(1, "Couldn't send result of calctask to client.\n");
}

void timetask (struct Msg *msger)
{
    int clientqueue = prepmsg(msger);
    if (clientqueue == -1) FAILURE_EXIT(1, "Couldn't find client's ID\n");
    char buff[20];
    time_t now = time(NULL);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    sprintf(msger->mtext, "%s", buff);
    if (msgsnd(clientqueue, msger, MAXMSG, 0) == -1) FAILURE_EXIT(1, "Couldn't send result of timetask to client.\n");
}

void removeclienttask(struct Msg *msger)
{
	int i =0;
	for (; i<MAXCLIENTS; i++)
    {
        if (clientsarray[i][0] == msger->pid) break;
    }
    for (; i <= clientscounter; i++)
    {
    	clientsarray[i][0] = clientsarray[i+1][0];
    	clientsarray[i][1] = clientsarray[i+1][1];
    }
    clientscounter--;
    printf("SERVER: Client successfully removed from clientsarray.\n");
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
