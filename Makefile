# Compiler and linker flags needed to link to the needed libraries:
CFLAGS = `pkg-config --cflags guile-3.0`
LDFLAGS= -lpthread -lncurses -lgnutls `pkg-config --libs guile-3.0`

# Files needed to compile the client:
clientsrc = $(wildcard src/*.c) src/client/SilverMUDClient.c
clientobj = $(clientsrc:.c=.o)

# Files needed to compile the server:
serversrc = $(wildcard src/*.c) src/server/SilverMUDServer.c
serverobj = $(serversrc:.c=.o)

# Default target: Compile the client and server with aggressive optimizations and a big stack of warnings:
all: clean SilverMUDClient SilverMUDServer
all: CFLAGS += -Wall -Wextra -Ofast

# Debug target: Compile the client and server with profiling, debug information, debug optimization, and the
# preprocessor flag "debug" set.
debug: CFLAGS += -Wall -Wextra -pg -ggdb -Og -D debug
debug: clean SilverMUDClientDebug SilverMUDServerDebug

SilverMUDClient: $(clientobj)
	cc $^ $(LDFLAGS) -o $@

SilverMUDServer: $(serverobj)
	cc $^ $(LDFLAGS) -o $@ 

SilverMUDClientDebug: $(clientobj)
	cc -pg $^ $(LDFLAGS) -o $@

SilverMUDServerDebug: $(serverobj)
	cc -pg $^ $(LDFLAGS) -o $@

# Start from a clean slate:
.PHONY: clean
clean:
	rm -f $(clientobj) $(serverobj) SilverMUDClient SilverMUDServer SilverMUDClientDebug SilverMUDServerDebug gmon.out

