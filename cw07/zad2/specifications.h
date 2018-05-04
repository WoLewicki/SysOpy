//
// Created by wojlewy on 26.04.18.
//

#ifndef ZAD2_SPECIFICATIONS_H
#define ZAD2_SPECIFICATIONS_H

#define MAXCLIENTS 1000


typedef struct shmstruct
{
    int seatlimit;
    int clientscounter;
    int asleep;
    int currentlyshaved;
    int seats[MAXCLIENTS];
}Shm;

const char *shmpath = "/shm";
const char *seatsarray = "/SEATSARRAYSEM";
const char *isasleep = "/ASLEEPSEM";
const char *wakemeup = "/WAKEMEUPSEM";
const char *sit = "/SITSEM";
const char *shaveme = "/SHAVEMESEM";
const char *shavingended = "/SHAVINGENDEDSEM";
const char *ileft = "/ILEFTSEM";
#endif //ZAD2_SPECIFICATIONS_H