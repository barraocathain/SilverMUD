// =========================================
// | SilverMUD Server - main.c             |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <libguile.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "scheme-integration.h"

static const int PORT = 5000;
static const int CONCURRENT_PLAYER_COUNT = 256;

int main (int argc, char ** argv)
{
	// Print a welcome message:
	printf("SilverMUD Server - Starting Now.\n"
		   "================================\n");

	// Initialize Scheme:
	scm_init_guile();

	// Start the REPL server on a UNIX socket:
	scm_c_eval_string("(begin (use-modules (system repl server))"
					  "(if (file-exists? \"silvermud-repl\") (delete-file \"silvermud-repl\"))"
					  "(spawn-server (make-unix-domain-server-socket #:path \"silvermud-repl\")))");

	// Create a socket to listen for connections on:
	int masterSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (masterSocket < 0)
	{
		fprintf(stderr, "Failed to create socket. Aborting.\n");
		exit(EXIT_FAILURE);
	}

	// Allow reusing the address so that quick re-launching doesn't fail:
	//setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	//setsockopt(masterSocket, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	
	// Setup the server address struct to bind the master socket to:
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(struct sockaddr_in));

	// Assign the IP address and port to the server address struct:
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);

	// Bind the master socket to the server address:
	if ((bind(masterSocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in))) != 0)
	{
		fprintf(stderr, "Failed to bind socket. Aborting.\n");
		exit(EXIT_FAILURE);
	}

	// Begin listening:
	if ((listen(masterSocket, CONCURRENT_PLAYER_COUNT)) != 0)
	{
		fprintf(stderr, "Failed to begin listening on the master socket. Aborting.\n");
		exit(EXIT_FAILURE);
	}

	// Create an epoll instance for managing connections, and add the master socket to it:
	int connectedClients = epoll_create(CONCURRENT_PLAYER_COUNT);
	if (connectedClients < 0)
	{
		fprintf(stderr, "Failed to create epoll instance. Aborting.\n");
		exit(EXIT_FAILURE);
	}
	struct epoll_event watchedEvents;
	watchedEvents.events = EPOLLIN;
	watchedEvents.data.fd = masterSocket;
	epoll_ctl(connectedClients, EPOLL_CTL_ADD, masterSocket, &watchedEvents);   

	int eventsCount = 0;
	struct epoll_event events[1024];

	// Start a REPL thread:
	pthread_t schemeREPLThread;
	pthread_create(&schemeREPLThread, NULL, schemeREPLHandler, NULL);

	while (true)
	{
		eventsCount = epoll_wait(connectedClients, events, 1024, -1);
		if (eventsCount == -1)
		{
			fprintf(stderr, "epoll_wait() failed. Aborting.\n");
			exit(EXIT_FAILURE);
		}
		for (int index = 0; index < eventsCount; index++)
		{
			// If it's the master socket, it's a new client connecting:
			if (events[index].data.fd == masterSocket)
			{
				int newSocket = accept(masterSocket, NULL, NULL);
				send(newSocket, "Hello, world!", 13, 0);
				close(newSocket);
			}
		}
	}
	
	// Wait for all other threads to terminate:
    pthread_join(schemeREPLThread, NULL);
	
	// Return a successful status code to the operating system:
	return 0;
}

// ============================================
// | End of main.c, copyright notice follows. |
// ============================================

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
