#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

int L;
int Type;
int child;
volatile int sigcounter =0;
volatile int parentcounter = 0;
volatile int childcounter = 0;


void childfirst (int signo, siginfo_t *info, void *context) // niewazne jaki Type zostal wybrany.
{
    if (signo == SIGRTMIN+5 || signo == SIGUSR1) {
        childcounter++;
        printf("Otrzymano %d sygnal od rodzica i wysylanie go z powrotem.\n", childcounter);
        signo == SIGRTMIN+5 ? kill(getppid(), SIGRTMIN+5) : kill(getppid(), SIGUSR1);
    }
}

void childsecond (int signo, siginfo_t *info, void *context)
{
    if (signo == SIGRTMIN+10 || signo == SIGUSR2) {
        childcounter++;
        printf("Otrzymano sygnal zakonczenia od rodzica.\n");
        printf("Odebrano lacznie %d sygnalow przez potomka.\n", childcounter);
        exit(0);
    }
}

void parentinthandler (int signo, siginfo_t *info, void *context)
{
    printf("Odebrano SIGINT. Konczenie pracy potomka i programu.\n");
    Type == 3 ? kill(child, SIGRTMIN+10) : kill(child, SIGUSR2);
}

void parentusr1handler (int signo, siginfo_t *info, void *context)
{
    if (signo == SIGRTMIN + 5 || signo == SIGUSR1) {
        parentcounter++;
        printf("Otrzymano %d sygnal od dziecka.\n", parentcounter);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 3)
    {
        printf("Podano zla liczbe argumentow.\n");
        exit(1);
    }
    L = (int) strtol(argv[1], NULL, 10);
    Type = (int) strtol(argv[2], NULL, 10);
    if (L < 1 || Type < 1 || Type > 3)
    {
        printf("Podano bledne argumenty.\n");
        exit(1);
    }

    child = fork();
    if (!child)
    {
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO;
        sigset_t mask;
        switch (Type)
        {
            case 3:
            {
                act.sa_sigaction = childfirst;
                if (sigaction(SIGRTMIN+5, &act, NULL) == -1)
                {
                    printf("Nie mozna przechwycic SIGRTMIN + 5.\n");
                    exit(1);
                }
                act.sa_sigaction = childsecond;
                if (sigaction(SIGRTMIN+10, &act, NULL) == -1)
                {
                    printf("Nie mozna przechwycic SIGRTMIN + 10.\n");
                    exit(1);
                }
                sigfillset(&mask);
                sigdelset(&mask, SIGRTMIN+5);
                sigdelset(&mask, SIGRTMIN+10);
                break;
            }
            default:
            {
                act.sa_sigaction = childfirst;
                if (sigaction(SIGUSR1, &act, NULL) == -1)
                {
                    printf("Nie mozna przechwycic SIGUSR1.\n");
                    exit(1);
                }
                act.sa_sigaction = childsecond;
                if (sigaction(SIGUSR2, &act, NULL) == -1)
                {
                    printf("Nie mozna przechwycic SIGUSR2.\n");
                    exit(1);
                }
                sigfillset(&mask);
                sigdelset(&mask, SIGUSR1);
                sigdelset(&mask, SIGUSR2);
                break;
            };
        }
        sigprocmask(SIG_SETMASK, &mask, NULL);
        while (1) sleep(1);
    } else
    {
        sleep(1); // poczekaj na ustalenie sie dzialania potomka
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = parentinthandler;
        if (sigaction(SIGINT, &act, NULL) == -1)
        {
            printf("Nie mozna przechwycic SIGINT w rodzicu.\n");
            exit(1);
        }
        if (Type == 1 || Type == 2)
        {
            act.sa_sigaction = parentusr1handler;
            if (sigaction(SIGUSR1, &act, NULL) == -1)
            {
                printf("Nie mozna przechwycic SIGUSR1 w rodzicu.\n");
                exit(1);
            }
            sigset_t mask;
            sigfillset(&mask);
            sigdelset(&mask, SIGUSR1);
            sigdelset(&mask, SIGINT);
            for(int i = 0; i < L; i++) {
                sigcounter++;
                printf("Wysylanie %d sygnalu SIGUSR1 do dziecka.\n", sigcounter);
                kill(child, SIGUSR1);
                if(Type == 2)
                    sigsuspend(&mask); // czekam az dziecko odesle mi SIGUSR1 albo SIGINT
            }
            printf("Wysylanie sygnalu SIGUSR2 do dziecka.\n");
            kill(child, SIGUSR2); // nie interesuje mnie czy dziecko odebralo juz wszystkie sygnaly
        } else{
            act.sa_sigaction = parentusr1handler; // tak naprawde pobieram SIGMIN + 5 ale dzialanie jest to samo
            if (sigaction(SIGRTMIN+5, &act, NULL) == -1)
            {
                printf("Nie mozna przechwycic SIGRTMIN+5 w rodzicu.\n");
                exit(1);
            }
            sigset_t mask;
            sigfillset(&mask);
            sigdelset(&mask, SIGRTMIN+5);
            sigdelset(&mask, SIGINT);
            for(int i = 0; i < L; i++)
            {
                sigcounter++;
                printf("Wysylanie %d sygnalu SIGRTMIN+5 do dziecka.\n", sigcounter);
                kill(child, SIGRTMIN+5);
            }
            printf("Wysylanie sygnalu SIRTMIN+10 do dziecka.\n");
            kill(child, SIGRTMIN+10);
        }
    }
    int status;
    waitpid(child, &status, 0);
    if (WEXITSTATUS(status) < 0)
    {
        printf("Zakonczono w nieporzadany sposob.\n");
        exit(1);
    }
    printf("Wyslano lacznie %d sygnalow do potomka.\n", sigcounter);
    printf("Odebrano lacznie %d sygnalow od potomka.\n", parentcounter);
    return 0;
}