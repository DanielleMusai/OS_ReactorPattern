CC = gcc

all: reactor server client

server: react_server.c 
	$(CC) -o react_server react_server.c -L. -lst_reactor -lpthread

client: react_client.c 
	$(CC) -o react_client react_client.c

reactor: reactor.c reactor.h
	$(CC) -c -fPIC reactor.c -o reactor.o
	$(CC) -shared -o libst_reactor.so reactor.o

clean:
	rm -rf *.o react_server *.so react_client
