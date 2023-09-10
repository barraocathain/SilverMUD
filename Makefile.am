bin_PROGRAMS = SilverMUDServer SilverMUDClient
dist_doc_DATA = README.org
SilverMUDServer_CFLAGS = -lgnutls -g $(GUILE_CFLAGS) $(GUILE_LIBS)
SilverMUDClient_CFLAGS = -lgnutls -g -lncurses $(GUILE_CFLAGS) $(GUILE_LIBS)

SilverMUDServer_SOURCES = \
	source/messages.c \
	source/server/player-data.c \
	source/server/connections.c \
	source/server/scheme-integration.c \
	source/server/main.c 

SilverMUDClient_SOURCES = \
	source/messages.c \
	source/client/client-drawing.c \
	source/client/receiving-thread.c \
	source/client/main.c
