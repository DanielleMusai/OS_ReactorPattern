#ifndef REACTOR_H
#define REACTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>


#define MAX_FDS 1024

typedef void (*handler_t)(int fd);

typedef struct {
    int fd;
    handler_t handler;
} FdHandlerPair;

typedef struct {
    int running;
    FdHandlerPair fdHandlers[MAX_FDS];
    int numFdHandlers;
    pthread_t thread;
} Reactor;

void* createReactor();

void stopReactor(void* this);

void* reactorThread(void* arg);

void startReactor(void* this);

void addFd(void* this, int fd, handler_t handler);

void WaitFor(void* this);

#endif /* REACTOR_H */
