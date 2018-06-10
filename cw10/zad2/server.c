#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/un.h>

#include "specifications.h"

void main_process_function();

int inet, local, epolld;
int expression_counter;

char *local_name;

pthread_mutex_t clients_array_mutex;
struct client clients_array[MAX_CLIENTS];

void cleanup()
{
    struct msg msg;
    msg.msg_type = END;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients_array[i].pings == 0)
        {
            sendto(clients_array[i].fd, &msg, sizeof(msg), 0, &clients_array[i].addr, clients_array[i].addr_size);
        }
    }
    pthread_mutex_destroy(&clients_array_mutex);
    close(epolld);
    close(inet);
    close(local);
    remove(local_name);
}

void inthandler(int signo)
{
    if (signo == SIGINT)
    {
        printf("Caught CTRL+C. Exiting.\n");
        exit(0);
    }
}

void close_connection(struct epoll_event events)
{
    pthread_mutex_lock(&clients_array_mutex);
    shutdown(events.data.fd, SHUT_RDWR);
    close(events.data.fd);
    for(int i=0; i<MAX_CLIENTS; i++)
    {
        if(clients_array[i].pings >=0 && events.data.fd == clients_array[i].fd)
        {
            clients_array[i].pings = -1;
            for(int j=0; j<MAX_LEN; j++) clients_array[i].name[j] = 0;
        }
    }
    pthread_mutex_unlock(&clients_array_mutex);
}

void handle_response (struct epoll_event events)
{
    struct msg msg;
    struct sockaddr *addr = malloc(sizeof(struct sockaddr));
    socklen_t addr_size = sizeof(struct sockaddr);
    ssize_t quantity = recvfrom(events.data.fd, &msg, sizeof(msg), 0, addr, &addr_size);
    if (quantity == 0)
    {
        printf("Ending connection with client %d.\n", events.data.fd);
        fflush(stdout);
        close_connection(events);
        return;
    } else
    {
        switch(msg.msg_type)
        {
            case START:
                pthread_mutex_lock(&clients_array_mutex);
                for(int i=0; i<MAX_CLIENTS; i++)
                {
                    if(clients_array[i].pings >=0 && strcmp(msg.name, clients_array[i].name) == 0) // there is such client
                    {
                        pthread_mutex_unlock(&clients_array_mutex);
                        sendto(events.data.fd, &msg, sizeof(msg), 0, &clients_array[i].addr, clients_array[i].addr_size); //sending back START message
                        close_connection(events);
                        return;
                    }
                }
                for(int i=0; i<MAX_CLIENTS; i++)
                {
                    if(clients_array[i].pings == -1)
                    {
                        printf("Starting connection with client %d with name %s.\n", events.data.fd, msg.name);
                        fflush(stdout);
                        strcpy(clients_array[i].name, msg.name);
                        clients_array[i].addr = *addr;
                        clients_array[i].addr_size = addr_size;
                        clients_array[i].fd = events.data.fd;
                        clients_array[i].pings =0;
                        pthread_mutex_unlock(&clients_array_mutex);
                        return;
                    }
                }
            case ORDER:
                expression_counter++;
                printf("Expression number: %d, from client %s, result: %d.\n", expression_counter, msg.name, msg.msg_order.operand1);
                fflush(stdout);
                return;

            case PING:
                pthread_mutex_lock(&clients_array_mutex);
                clients_array[msg.id].pings = 0;
                pthread_mutex_unlock(&clients_array_mutex);
                return;
            default:
                break;
        }
    }
}

void *do_math(void *args)
{
    for (int i =0; i<MAX_CLIENTS; i++) clients_array[i].pings = -1;
    struct epoll_event events[MAX_EVENTS];
    while(1)
    {
        int counter = epoll_wait(epolld, events, MAX_EVENTS, -1); // waiting infinitely
        for (int i=0; i<counter; i++) // there is no connection now
        {
                handle_response(events[i]);
        }
    }
}

void *ping(void *args)
{
    sleep(3);
    while(1)
    {
        struct msg msg;
        msg.msg_type = PING;
        pthread_mutex_lock(&clients_array_mutex);
        for(int i=0; i< MAX_CLIENTS; i++)
        {
            if(clients_array[i].pings == 0)
            {
                clients_array[i].pings =1;
                msg.id =i;
                sendto(clients_array[i].fd, &msg, sizeof(msg), 0, &clients_array[i].addr, clients_array[i].addr_size);
            }
        }
        sleep(3); // waiting for all clients to response by decremeting their pings counter
        for (int i = 0; i <MAX_CLIENTS ; ++i) {
            if(clients_array[i].pings == 1) // didn't decrement so is not responding
            {
                clients_array[i].pings = -1;
                for (int j = 0; j <MAX_LEN ; ++j) {
                    clients_array[i].name[j] =0;
                }
            }
        }
        pthread_mutex_unlock(&clients_array_mutex);
        sleep(10);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        FAILURE_EXIT(1, "Pass 2 arguments\n");
    }
    atexit(cleanup);
    signal(SIGINT, inthandler);
    pthread_mutex_init(&clients_array_mutex, NULL);
    unsigned short port = (unsigned short) strtoul(argv[1], NULL, 10);
    local_name = argv[2];



    // inet connection
    inet = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(inet, (struct sockaddr *)&(addr), sizeof(addr)) != 0) {
        FAILURE_EXIT(1, "Couldn't bind in INET.\n");
    }

    epolld = epoll_create1(0);
    struct epoll_event e;
    e.events = EPOLLIN | EPOLLET;
    e.data.fd = inet;
    if (epoll_ctl(epolld, EPOLL_CTL_ADD, inet, &e) < 0)
    {
        FAILURE_EXIT(1, "Couldn't start epoll for INET.\n");
    }

    //local connection
    local = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, local_name);

    if (bind(local, (struct sockaddr *)&(address), sizeof(address)) != 0){
        FAILURE_EXIT(1, "Couldn't bind in LOCAL.\n");
    }

    e.events = EPOLLIN | EPOLLET;
    e.data.fd = local;
    if (epoll_ctl(epolld, EPOLL_CTL_ADD, local, &e) <0){
        FAILURE_EXIT(1, "Couldn't start epoll for LOCAL.\n");
    }

    pthread_t math_thread;
    pthread_create(&math_thread, NULL, do_math, NULL);

    pthread_t pinger_thread;
    pthread_create(&pinger_thread, NULL, ping, NULL);



    main_process_function();
    return 0;
}


void main_process_function()
{
    sleep(3);
    int operation;
    int operand1;
    int operand2;
    while(1)
    {
        printf("Pass operation (ADD - 5, SUB - 6, MUL - 7, DIV - 8), then after space first and second operand :\n");
        int how_much = scanf("%d %d %d", &operation, &operand1, &operand2);
        if (how_much != 3)
        {
            printf("Press enter and pass good operation.\n");
            char c;
            while((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        struct msg msg;
        msg.msg_type = ORDER;
        msg.msg_order.operation = operation;
        msg.msg_order.operand1 = operand1;
        msg.msg_order.operand2 = operand2;
        int infinity =1;
        while(infinity) {
            pthread_mutex_lock(&clients_array_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients_array[i].pings == 0) {
                    ssize_t quantity = sendto(clients_array[i].fd, &msg, sizeof(msg), 0, &clients_array[i].addr, clients_array[i].addr_size);
                    if (quantity <= 0) {
                        printf("Couldn't send msg to client %s.\n", clients_array[i].name);
                        fflush(stdout);
                    } else {
                        printf("Sending order to client %s.\n", clients_array[i].name);
                        fflush(stdout);
                        infinity = 0;
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&clients_array_mutex);
        }
    }
}