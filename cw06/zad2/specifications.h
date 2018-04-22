//
// Created by wojlewy on 20.04.18.
//
#ifndef ZAD1_SPECIFICATIONS_H
#define ZAD1_SPECIFICATIONS_H

#include <unistd.h>

#define MAXMSGLENGTH 1024
#define MAXMSG sizeof(Msg)
#define MAXCLIENTS 10
#define MSGSLIMIT 9

typedef enum mtype{
    START = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, CLOSE =6
} mtype;


typedef struct Msg {
    int mtype;
    pid_t pid;
    char mtext[MAXMSGLENGTH];
} Msg;

const char serverpath[] = "/server";

#endif //ZAD2_SPECIFICATIONS_H