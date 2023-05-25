
#ifndef REACTOR_H
#define REACTOR_H
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#define PORT "9034"     // Port we're listening on
#define MAX_CLIENTS 100 // Max number of people on chat
#define BUFF_SIZE 256   // Max buffer size

#include <sys/poll.h>
#include <pthread.h>

typedef struct reactor *p_reactor_t;

typedef struct
{
    void (*handler)(p_reactor_t, int, void *);
    void *arg;
} handler_t, *p_handler_t;

typedef struct reactor
{
    p_handler_t *handlers;
    struct pollfd *fds;
    int count;
    int size;
    int isRunning;
    int listenerFd;
    char *remoteIP;
    pthread_t thread;
} reactor_t, *p_reactor_t;

p_reactor_t createReactor(int size, int listenerFd);
void stopReactor(p_reactor_t reactor);
void startReactor(p_reactor_t reactor);
void *runReactor(void *arg);
void addFd(p_reactor_t reactor, int fd, handler_t handler);
void waitFor(p_reactor_t reactor);
void deleteReactor(p_reactor_t reactor);
void deleteFd(p_reactor_t reactor, int fd);
void sigHandler(int sig);
void *get_in_addr(struct sockaddr *sa);
int get_listener_socket(void);
void connectionHandler(struct reactor *reactor, void *arg);
void clientHandler(p_reactor_t reactor, int client_fd, void *arg);
void gotAnError();

#endif // REACTOR_H
