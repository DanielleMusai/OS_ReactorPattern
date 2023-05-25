#include "reactor.h"

p_reactor_t p_reactor;

void gotAnError()
{
    perror("Error");
    exit(1);
}
void sigHandler(int sig)
{
    if (sig == SIGINT)
    {
       deleteReactor(p_reactor);
        exit(0);
    }
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// return a listening socket
int get_listener_socket(void)
{
    int listener; // listening socket descriptor
    int yes = 1;  // for setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        gotAnError();
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }
    freeaddrinfo(ai);
    printf("Server is up and run on 9034 port\n");

    // if we got here, it means we didn't get bound
    if (p == NULL)
    {
        exit(2);
    }

    // listen
    if (listen(listener, MAX_CLIENTS) == -1)
    {

        exit(3);
    }

    return listener;
}

void connectionHandler(struct reactor *reactor, void *arg)
{
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    int newfd;
    int listener = reactor->listenerFd;
    addrlen = sizeof remoteaddr;
    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen); // accept the incoming connection
    if (newfd == -1)
    {
        perror("accept");
    }
    else
    {
        handler_t cHandler;
        cHandler.arg = NULL;
        cHandler.handler = &clientHandler;
        addFd(reactor, newfd, cHandler);
    }
}

void clientHandler(p_reactor_t reactor, int client_fd, void *arg)
{
    char buf[BUFF_SIZE] = {0};
    int nbytes;
    nbytes = recv(client_fd, buf, BUFF_SIZE, 0);
    if (nbytes <= 0)
    {
        if (nbytes == 0)
        {
            printf("server: socket %d hung up\n", client_fd);
        }
        else
        {
            perror("recv");
        }
        close(client_fd);
        deleteFd(reactor, client_fd);
    }
    else
    {
        // send to all clients
        printf("server: socket %d got message: %s\n", client_fd, buf);
        for (int i = 0; i < reactor->count; i++)
        {
            if (reactor->fds[i].fd != client_fd && reactor->fds[i].fd != reactor->listenerFd)
            {
                if (send(reactor->fds[i].fd, buf, nbytes, 0) == -1)
                {
                    perror("send");
                }
            }
        }

        printf("message sent to all clients\n");
    }
}

int main(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    int listener;
    int newfd;
    int sizeR = 10;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];

    // Set up and get a listening socket
    printf("waiting for connections...\n");
    listener = get_listener_socket();
    if (listener == -1)
    {
        fprintf(stderr, "error\n");
        exit(1);
    }
    // Create reactor
    p_reactor = createReactor(sizeR, listener);
    handler_t cHandler; 
    cHandler.arg = NULL;
    cHandler.handler = &connectionHandler;
    addFd(p_reactor, listener, cHandler);
    startReactor(p_reactor);
    printf("Reactor is running for stopping the programm use CTRL+C\n");
    while (p_reactor->isRunning)
    {
        waitFor(p_reactor);
    }

    return 0;
}