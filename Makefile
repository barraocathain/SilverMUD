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
	gcc  $^ $(CLIENTLDFLAGS) -o $@

SilverMUDServerDebug: $(serverobj)
	gcc  $^ $(SERVERLDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(clientobj) $(serverobj) SilverMUDClient SilverMUDServer SilverMUDClientDebug SilverMUDServerDebug

all: SilverMUDClient SilverMUDServer
all: CFLAGS += -Wall -Wextra -Ofast
debug: CFLAGS += -Wall -ggdb -Wextra -Og -pg
debug: clean SilverMUDClientDebug SilverMUDServerDebug
