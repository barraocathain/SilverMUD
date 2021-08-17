CC = gcc
clientsrc = $(wildcard src/misc/*.c) \
	    src/SilverMUDClient.c
clientobj = $(clientsrc:.c=.o) 
serversrc = $(wildcard src/misc/*.c) \
	    src/SilverMUDServer.c
serverobj = $(serversrc:.c=.o) 
CLIENTLDFLAGS= -lpthread

SilverMUDClient: $(clientobj)
	gcc -o $@ $^ $(CLIENTLDFLAGS)

SilverMUDServer: $(serverobj)
	gcc -o $@ $^

.PHONY: clean
clean:
	rm -f $(clientobj) $(serverobj) SilverMUDClient SilverMUDServer

all: SilverMUDClient SilverMUDServer
