CC = gcc
clientsrc = $(wildcard src/misc/*.c) \
	    src/SilverMUDClient.c
clientobj = $(clientsrc:.c=.o) 
serversrc = $(wildcard src/misc/*.c) \
	    src/SilverMUDServer.c
serverobj = $(serversrc:.c=.o) 
CLIENTLDFLAGS= -lpthread -lncurses -lgnutls
SERVERLDFLAGS= -lpthread -lncurses -lgnutls 
SilverMUDClient: $(clientobj)
	gcc -s -O3 -o $@ $^ $(CLIENTLDFLAGS)

SilverMUDServer: $(serverobj)
	gcc -s -O3 -o $@ $^ $(SERVERLDFLAGS)

SilverMUDClientDebug: $(clientobj)
	gcc -ggdb -Wall $^ $(CLIENTLDFLAGS) -o $@

SilverMUDServerDebug: $(serverobj)
	gcc -ggdb -Wall $^ $(SERVERLDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(clientobj) $(serverobj) SilverMUDClient SilverMUDServer SilverMUDClientDebug SilverMUDServerDebug

all: SilverMUDClient SilverMUDServer

debug: CFLAGS += -Wall -ggdb
debug: SilverMUDClientDebug SilverMUDServerDebug
