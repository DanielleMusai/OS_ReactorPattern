#include "reactor.h"
#include <stdlib.h>

p_reactor_t createReactor(int size, int listenerFd)
{
    p_reactor_t reactor = (p_reactor_t)malloc(sizeof(reactor_t));
    reactor->handlers = (p_handler_t *)malloc(size * sizeof(p_handler_t));
    reactor->fds = (struct pollfd *)malloc(size * sizeof(struct pollfd));
      if(reactor->handlers == NULL || reactor->fds == NULL)
        {
            perror("");
            exit(1);
        }
    reactor->count = 0;
    reactor->size = size;
    reactor->isRunning = 0;
    reactor->listenerFd = listenerFd;
    return reactor;
}

void stopReactor(p_reactor_t reactor)
{
    if (reactor->isRunning == 1)
    {
        reactor->isRunning = 0;
        waitFor(reactor);
    }
 
}

void startReactor(p_reactor_t reactor)
{
    if (reactor->isRunning == 1)
    {
        return;
    }
    reactor->isRunning = 1;

    if (pthread_create(&reactor->thread, NULL, runReactor, reactor) != 0)
    {
        gotAnError();
    }
}

void *runReactor(void *arg)
{
    p_reactor_t reactor = (p_reactor_t)arg;
    while (reactor->isRunning == 1)
    {
        int numEvents = poll(reactor->fds, reactor->count, -1);
        if (numEvents > 0)
        {
            for (int i = 0; i < reactor->count; i++)
            {
                if (reactor->fds[i].revents & POLLIN)
                {
                    reactor->handlers[i]->handler(reactor, reactor->fds[i].fd, reactor->handlers[i]->arg);
                }
            }
        }
    }
}

void addFd(p_reactor_t reactor, int fd, handler_t handler)
{

    if (reactor->listenerFd == 0)
    {
        reactor->listenerFd = fd;
    }
    if (reactor->count >= reactor->size)
    {
        reactor->size *= 2;
        reactor->handlers = (p_handler_t *)realloc(reactor->handlers, reactor->size * sizeof(p_handler_t));
        reactor->fds = (struct pollfd *)realloc(reactor->fds, reactor->size * sizeof(struct pollfd));
        if(reactor == NULL || reactor->fds == NULL)
        {
            perror("");
            exit(1);
        }
    }
    else
    {
        reactor->handlers[reactor->count] = (p_handler_t)malloc(sizeof(handler_t));
        reactor->handlers[reactor->count]->handler = handler.handler;
        reactor->fds[reactor->count].fd = fd;
        reactor->fds[reactor->count].events = POLLIN;
        reactor->count++;
    }
}

void waitFor(p_reactor_t reactor)
{
   // if (reactor && !reactor->isRunning)
        pthread_join(reactor->thread, NULL);
}

void deleteFd(p_reactor_t reactor, int fd)
{
    int index = -1;
    for (int i = 0; i < reactor->count; i++)
    {
        if (reactor->fds[i].fd == fd)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        free(reactor->handlers[index]);
        for (int i = index; i < reactor->count - 1; i++)
        {
            reactor->handlers[i] = reactor->handlers[i + 1];
            reactor->fds[i] = reactor->fds[i + 1];
        }
        reactor->count--;
    }
}

void deleteReactor(p_reactor_t reactor)
{
    if (reactor == NULL)
    {
        return;
    }

    if (reactor->isRunning)
    {
        stopReactor(reactor);
    }

    if (reactor->handlers != NULL)
    {
        for (int i = 0; i < reactor->count; i++)
        {
            free(reactor->handlers[i]);
        }
        free(reactor->handlers);
    }

    if (reactor->fds != NULL)
    {
        free(reactor->fds);
    }

    free(reactor);
}
