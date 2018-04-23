//
// Created by wojlewy on 20.04.18.
//
#ifndef ZAD1_SPECIFICATIONS_H
#define ZAD1_SPECIFICATIONS_H

#define MAXMSG 256
#define ID 101

#define MAXCLIENTS 10


typedef enum mtype{
    START = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, STOP = 6
} mtype;


typedef struct Msg {
    int mtype;
    pid_t pid;
    char mtext[MAXMSG];
} Msg;

#endif //ZAD1_SPECIFICATIONS_H
