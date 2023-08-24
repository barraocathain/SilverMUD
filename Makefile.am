bin_PROGRAMS = SilverMUDServer SilverMUDClient
dist_doc_DATA = README.org
SilverMUDServer_CFLAGS = -I/usr/include/guile/3.0 -I/usr -lguile-3.0 -lgc -lpthread -ldl -lgnutls -g
SilverMUDClient_CFLAGS = -I/usr/include/guile/3.0 -I/usr -lguile-3.0 -lgc -lpthread -ldl -lgnutls -g

SilverMUDServer_SOURCES = \
	source/server/connections.c \
	source/server/scheme-integration.c \
	source/server/main.c 

SilverMUDClient_SOURCES = \
	source/client/main.c
