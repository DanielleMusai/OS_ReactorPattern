#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "reactor.h"
#define MAXDATASIZE 100
#define PORT "9034"   // port we're listening on

void handleNewConnection(int fd);
void handleDataFromClient(int fd);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int listener;     // listening socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Create a reactor instance
    void* reactor = createReactor();

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }
    
    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // Add the listener to the reactor
    addFd(reactor, listener, &handleNewConnection);

    // Main event loop
    while (1) {
        // Run the reactor
        startReactor(reactor);
    }

    // Cleanup
    stopReactor(reactor);

    return 0;
}

void handleNewConnection(int fd)
{
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;
    int newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfd == -1) {
        perror("accept");
        return;
    }

    printf("New connection on socket %d\n", newfd);

    // Handle the new connection
    // ...

    close(newfd);
}

void handleDataFromClient(int fd)
{
    char buffer[MAXDATASIZE];
    int nbytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (nbytes <= 0) {
        // Connection closed or error occurred
        if (nbytes == 0) {
            // Connection closed
            printf("Client on socket %d disconnected\n", fd);
        } else {
            perror("recv");
        }

        // Close the client socket
        close(fd);
        return;
    }

    buffer[nbytes] = '\0';
    printf("Received data from client on socket %d: %s\n", fd, buffer);

    // Process the received data
    // ...

    // Send a response back to the client if needed
    // ...
}
