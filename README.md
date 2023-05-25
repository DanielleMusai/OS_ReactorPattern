# OS_ReactorPattern
This repository contains the implementation of a Reactor pattern for handling network connections. The Reactor pattern is a design pattern used for asynchronous event handling and dispatching in event-driven systems.

## Introduction
The Reactor pattern allows you to handle multiple network connections concurrently by using a single thread. It utilizes the poll system call to efficiently monitor multiple file descriptors for events. This implementation provides a simple interface to create and manage a reactor instance, register event handlers, and run the reactor loop.

# API
The following functions are available for using the reactor:

p_reactor_t createReactor(int size, int listenerFd): Creates a new reactor instance with the specified size and listener file descriptor.

void startReactor(p_reactor_t reactor): Starts the reactor loop. This function spawns a new thread to run the reactor loop.

void stopReactor(p_reactor_t reactor): Stops the reactor loop. This function waits for the reactor thread to exit.

void addFd(p_reactor_t reactor, int fd, handler_t handler): Adds a new file descriptor and its associated event handler to the reactor.

void deleteFd(p_reactor_t reactor, int fd): Deletes a file descriptor from the reactor.

void deleteReactor(p_reactor_t reactor): Deletes the reactor instance and releases all associated resources.

void waitFor(p_reactor_t reactor): Waits for the reactor thread to exit.

# To run the project, follow these steps:

Build the project by running the following command:


* make all
Set the LD_LIBRARY_PATH environment variable to include the current directory. This is necessary to ensure that the dynamic linker can find the required libraries. Run the following command:


* export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
Execute the server application by running the following command:


* ./react_server
The server will start running and listening for incoming connections on port 9034. You can now connect to the server and test its functionality. To stop the server, use CTRL+C to send an interrupt signal.

<img width="835" alt="image" src="https://github.com/DanielleMusai/OS_ReactorPattern/assets/92378800/ba8e55ad-0e6e-4939-b98a-5cace12ddc84">
