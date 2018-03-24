#include <sys/stat.h>
#include <values.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <memory.h>
#include <dirent.h>
#include <unistd.h>


void getfilepermissions (char *pathname)
{
    struct stat fileStat;
    if(stat(pathname,&fileStat) < 0) exit(1);
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
}

char *getabsolutepath (char *pathname)
{
    char actualpath [PATH_MAX+1];
    return realpath(pathname, actualpath);
}

void printinfo(char *pathname, char *argument, long modtime)
{
    struct stat attrib;
    stat(pathname, &attrib);
    if (strcmp(argument, "<") == 0) {
        if (attrib.st_mtime < modtime) {
            char time[50];
            strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime)); // getting last modified time
            printf("Sciezka bezwgledna pliku: %s\n", pathname);
            printf("Rozmiar w bajtach: %d\n", (int) attrib.st_size);
            printf("Prawa dostepu do pliku :   ");
            getfilepermissions(pathname);
            printf("\n");
            printf("Data ostatniej modyfikacji pliku: %s\n", time);
        }
    }
    else if (strcmp(argument, "=") == 0) {
        if (attrib.st_mtime == modtime) {
            char time[50];
            strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime)); // getting last modified time
            printf("Sciezka bezwgledna pliku: %s\n", pathname);
            printf("Rozmiar w bajtach: %d\n", (int) attrib.st_size);
            printf("Prawa dostepu do pliku :   ");
            getfilepermissions(pathname);
            printf("\n");
            printf("Data ostatniej modyfikacji pliku: %s\n", time);
        }
    }
    else if (strcmp(argument, ">") == 0) {
        if (attrib.st_mtime > modtime) {
            char time[50];
            strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime)); // getting last modified time
            printf("Sciezka bezwgledna pliku: %s\n", pathname);
            printf("Rozmiar w bajtach: %d\n", (int) attrib.st_size);
            printf("Prawa dostepu do pliku :   ");
            getfilepermissions(pathname);
            printf("\n");
            printf("Data ostatniej modyfikacji pliku: %s\n", time);
        }
    }
}

void searchdir (char *pathname, char *argument, long modtime)
{
    printf("Entering new directory. Process ID: %d \n", (int) getpid());
    DIR *folder = opendir(pathname);
    if (folder == NULL)
    {
        printf("Wrong directory passed");
        exit(1);
    }
    struct dirent* file;
    while ((file = readdir(folder)))
    {
        if(file->d_type != DT_LNK && strcmp(file->d_name, ".")!= 0 && strcmp(file->d_name, "..") != 0)
        {
            if (file->d_type == DT_REG)
            {
                char *absolutepath = calloc(1000, sizeof(char));
                strcat(absolutepath, getabsolutepath(pathname)); //making absolute path to file
                strcat(absolutepath, "/");
                strcat(absolutepath, file->d_name);
                printinfo(absolutepath, argument, modtime);
            }
            else if (file->d_type == DT_DIR)
            {
                pid_t newprocess;
                if ((newprocess = fork()) == 0)
                {
                char *pathtosearchon = calloc(1000, sizeof(char));
                strcat(pathtosearchon, getabsolutepath(pathname)); //making absolute path to file
                strcat(pathtosearchon, "/");
                strcat(pathtosearchon, file->d_name);
                printf("%s\n",pathtosearchon);
                searchdir(pathtosearchon, argument, modtime);
                exit(0); // nie chcemy zeby proces dziecko dalej wykonywal instrukcje rodzica
                }
            }
        }
    }
}

int main (int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Zla liczba argumentow. Podaj 3");
        exit(1);
    }
    long modtime = strtol(argv[3],NULL,10);
    char buff[20];
    time_t data = modtime;
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&data));
    printf("%s - data\n",buff);
    searchdir(argv[1], argv[2], modtime);

    return 0;
}
