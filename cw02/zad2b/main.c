
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <values.h>

char *operator;
long modtime;
const char *getabsolutepath (const char *pathname)
{
    char actualpath [PATH_MAX+1];
    return realpath(pathname, actualpath);
}
void getfilepermissions (const char *pathname)
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
int printinfofornftw(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    if (tflag == FTW_F)
    {

        if (strcmp(operator, "<") == 0) {
            if (sb->st_mtime < modtime) {
                char time[50];
                strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&sb->st_mtime)); // getting last modified time
                printf("Sciezka bezwgledna pliku: %s\n", getabsolutepath(fpath));
                printf("Rozmiar w bajtach: %d\n", (int) sb->st_size);
                printf("Prawa dostepu do pliku :   ");
                getfilepermissions(fpath);
                printf("\n");
                printf("Data ostatniej modyfikacji pliku: %s\n", time);
            }
        }
        else if (strcmp(operator, "=") == 0) {
            if (sb->st_mtime == modtime) {
                char time[50];
                strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&sb->st_mtime)); // getting last modified time
                printf("Sciezka bezwgledna pliku: %s\n", getabsolutepath(fpath));
                printf("Rozmiar w bajtach: %d\n", (int) sb->st_size);
                printf("Prawa dostepu do pliku :   ");
                getfilepermissions(fpath);
                printf("\n");
                printf("Data ostatniej modyfikacji pliku: %s\n", time);
            }
        }
        else if (strcmp(operator, ">") == 0) {
            if (sb->st_mtime > modtime) {
                char time[50];
                strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&sb->st_mtime)); // getting last modified time
                printf("Sciezka bezwgledna pliku: %s\n", getabsolutepath(fpath));
                printf("Rozmiar w bajtach: %d\n", (int) sb->st_size);
                printf("Prawa dostepu do pliku :   ");
                getfilepermissions(fpath);
                printf("\n");
                printf("Data ostatniej modyfikacji pliku: %s\n", time);
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4)
    {
        printf("Zla liczba argumentow. Podaj 3");
        exit(1);
    }
    operator = argv[2];
    modtime = strtol(argv[3], NULL, 10);
    char buff[20];
    time_t data = modtime;
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&data));
    printf("%s - data\n",buff);
    nftw(argv[1], printinfofornftw, 100, FTW_PHYS);
    return 0;
}