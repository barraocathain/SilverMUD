CC = gcc
clientsrc = $(wildcard src/*.c) \
	    src/client/SilverMUDClient.c
clientobj = $(clientsrc:.c=.o) 
serversrc = $(wildcard src/*.c) \
	    src/server/SilverMUDServer.c
serverobj = $(serversrc:.c=.o) 
CLIENTLDFLAGS= -lpthread -lncurses -lgnutls
SERVERLDFLAGS= -lpthread -lncurses -lgnutls 
SilverMUDClient: $(clientobj)
	gcc  -o $@ $^ $(CLIENTLDFLAGS)

SilverMUDServer: $(serverobj)
	gcc  -o $@ $^ $(SERVERLDFLAGS)

SilverMUDClientDebug: $(clientobj)
	gcc -pg   $^ $(CLIENTLDFLAGS) -o $@

SilverMUDServerDebug: $(serverobj)
	gcc -pg   $^ $(SERVERLDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(clientobj) $(serverobj) SilverMUDClient SilverMUDServer SilverMUDClientDebug SilverMUDServerDebug

all: clean SilverMUDClient SilverMUDServer
all: CFLAGS += -Wall -Wextra -Ofast
debug: CFLAGS += -Wall -Wextra -pg -ggdb -Og
debug: clean SilverMUDClientDebug SilverMUDServerDebug
