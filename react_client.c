#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "reactor.c"

#define PORT "9034" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

void handleUserInput(int sockfd);
void handleDataFromServer(int sockfd);

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr* sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[])
{
int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
char s[INET6_ADDRSTRLEN];

if (argc != 2) {
    fprintf(stderr, "usage: client hostname\n");
    exit(1);
}

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;

if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
}

// loop through all the results and connect to the first we can
for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("client: socket");
        continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("client: connect");
        continue;
    }

    break;
}

if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
}

inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
printf("client: connecting to %s\n", s);

freeaddrinfo(servinfo); // all done with this structure

// Create a reactor instance
Reactor* reactor = (Reactor*)createReactor();

// Add the server socket to the reactor
addFd(reactor, sockfd, (handler_t)handleDataFromServer);

// Add the standard input to the reactor
addFd(reactor, STDIN_FILENO, (handler_t)handleUserInput);

// Main event loop
startReactor(reactor);

// Cleanup
stopReactor(reactor);

return 0;
}
void handleUserInput(int sockfd)
{
char buf[MAXDATASIZE];
int nbytes = read(STDIN_FILENO, buf, sizeof(buf));
if (nbytes <= 0) {
    // Error or end of file, exit the program
    if (nbytes == 0) {
        printf("stdin: end of file reached\n");
    } else {
        perror("read");
    }

    // Close the client socket
    close(sockfd);

    // Exit the program
    exit(0);
} else {
    // Send the user input to the server
    if (send(sockfd, buf, nbytes, 0) == -1) {
        perror("send");
    }
}
}

void handleDataFromServer(int sockfd)
{
char buf[MAXDATASIZE];
int nbytes = recv(sockfd, buf, sizeof(buf) - 1, 0);
if (nbytes <= 0) {
    // Connection closed or error occurred
    if (nbytes == 0) {
        // Connection closed
        printf("server: connection closed\n");
    } else {
        perror("recv");
    }

    // Close the client socket
    close(sockfd);

    // Exit the program
    exit(0);
} else {
    // We received some data from the server
    buf[nbytes] = '\0';
    printf("Received from server: %s\n", buf);
}
}