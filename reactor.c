#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include "reactor.h"
#define MAX_FDS 1024

void* createReactor() {
    Reactor* reactor = (Reactor*)malloc(sizeof(Reactor));
      if (reactor == NULL) {
        fprintf(stderr, "Failed to allocate memory for the reactor.\n");
        return NULL;
    }
    reactor->running = 0;
    reactor->numFdHandlers = 0;
    return reactor;
}

void stopReactor(void* this) {
    Reactor* reactor = (Reactor*)this;
    if (reactor->running) {
        reactor->running = 0;
    }
}

void* reactorThread(void* arg) {
    Reactor* reactor = (Reactor*)arg;
    startReactor(reactor);
    return NULL;
}

void startReactor(void* this) {
    Reactor* reactor = (Reactor*)this;
    reactor->running = 1;
    
    while (reactor->running) {
        fd_set readfds;
        int maxfd = 0;
        
        FD_ZERO(&readfds);
        
        for (int i = 0; i < reactor->numFdHandlers; i++) {
            int fd = reactor->fdHandlers[i].fd;
            
            FD_SET(fd, &readfds);
            
            if (fd > maxfd) {
                maxfd = fd;
            }
        }
        
        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        
        if (activity == -1) {
            perror("select");
            continue;
        }
        
        for (int i = 0; i < reactor->numFdHandlers; i++) {
            int fd = reactor->fdHandlers[i].fd;
            
            if (FD_ISSET(fd, &readfds)) {
                handler_t handler = reactor->fdHandlers[i].handler;
                handler(fd);
            }
        }
    }
}

void addFd(void* this, int fd, handler_t handler) {
    Reactor* reactor = (Reactor*)this;
    
    if (reactor->numFdHandlers >= MAX_FDS) {
        fprintf(stderr, "Maximum number of FDs reached.\n");
        return;
    }
    
    FdHandlerPair pair;
    pair.fd = fd;
    pair.handler = handler;
    reactor->fdHandlers[reactor->numFdHandlers] = pair;
    reactor->numFdHandlers++;
}

void WaitFor(void* this) {
    Reactor* reactor = (Reactor*)this;
    if (reactor->running) {
        pthread_join(reactor->thread, NULL);
    }
}
