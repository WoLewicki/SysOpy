//
// Created by wojlewy on 26.04.18.
//

#ifndef ZAD1_SPECIFICATIONS_H
#define ZAD1_SPECIFICATIONS_H
#define PROJ_ID 2018
#define MAXCLIENTS 1000
#define SEATSARRAYSEM 0
#define ASLEEPSEM 1
#define WAKEMEUPSEM 2
#define SITSEM 3
#define SHAVEMESEM 4
#define SHAVINGENDEDSEM 5
#define ILEFTSEM 6

typedef struct shmstruct
{
    int seatlimit;
    int clientscounter;
    int asleep;
    int currentlyshaved;
    int seats[MAXCLIENTS];
}Shm;

const char *semaphorepath = "./";
const char *shmpath = "/";

#endif //ZAD1_SPECIFICATIONS_H
