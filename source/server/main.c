// =========================================
// | SilverMUD Server - main.c             |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <errno.h>
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
#include <gnutls/gnutls.h>

#include "output-queue.h"
#include "player-data.h"
#include "connections.h"
#include "../messages.h"
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
	setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	setsockopt(masterSocket, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	
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

	// Setup the epoll events we want to watch for:
	struct epoll_event watchedEvents;
	watchedEvents.events = EPOLLIN;
	watchedEvents.data.fd = masterSocket;
	epoll_ctl(connectedClients, EPOLL_CTL_ADD, masterSocket, &watchedEvents);   

	int eventsCount = 0;
	struct epoll_event events[1024];

	// Setup the needed anonymous certificate for TLS:
	gnutls_global_init();
	gnutls_anon_server_credentials_t serverKey;
	gnutls_anon_allocate_server_credentials(&serverKey);
	gnutls_anon_set_server_known_dh_params(serverKey, GNUTLS_SEC_PARAM_MEDIUM);
	
	// Create a client connection list to allow us to associate TLS sessions and sockets and players:
	struct ClientConnectionList clientConnections;

	// Create some structures needed to store global state:
	struct PlayerList * globalPlayerList = createPlayerList();
	struct OutputQueue * globalOutputQueue = createOutputQueue();
	// Start a REPL thread:
	//pthread_t schemeREPLThread;
	//pthread_create(&schemeREPLThread, NULL, schemeREPLHandler, NULL);

	while (true)
	{
		do
		{
			eventsCount = epoll_wait(connectedClients, events, 1024, -1);
		} while (eventsCount < 0 && errno == EINTR);
		
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
				// Setup a TLS Session:
				gnutls_session_t * tlsSession = calloc(1, sizeof(gnutls_session_t));				
				gnutls_init(tlsSession, GNUTLS_SERVER);
				gnutls_priority_set_direct(*tlsSession, "NORMAL:+ANON-ECDH:+ANON-DH", NULL);
				gnutls_credentials_set(*tlsSession, GNUTLS_CRD_ANON, serverKey);
				gnutls_handshake_set_timeout(*tlsSession, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

				// Accept the connection:
				int newSocket = accept(masterSocket, NULL, NULL);
				gnutls_transport_set_int(*tlsSession, newSocket);

				// Perform a TLS handshake:
				int handshakeReturnValue = 0;
				do 
				{
					handshakeReturnValue = gnutls_handshake(*tlsSession);
				} while (handshakeReturnValue < 0 && gnutls_error_is_fatal(handshakeReturnValue) == 0);

				// If the handshake was unsuccessful, close the connection:
				if (handshakeReturnValue < 0)
				{
					printf("%d", handshakeReturnValue);
					fflush(stdout);
					gnutls_bye(*tlsSession, 2);
					shutdown(newSocket, 2);
					close(newSocket);
					break;
				}			   

				// Setup the epoll events we want to watch for:				
				watchedEvents.events = EPOLLIN;
				watchedEvents.data.fd = newSocket;			   
				epoll_ctl(connectedClients, EPOLL_CTL_ADD, newSocket, &watchedEvents);

				// Add the connection to the list:
				struct ClientConnection * newConnection = addNewConnection(&clientConnections, newSocket, tlsSession);
				newConnection->player = createNewPlayer(newConnection);
				sprintf(newConnection->player->name, "Player %02d", globalPlayerList->count + 1);
				addToPlayerList(newConnection->player, globalPlayerList);

				// Prepare a welcome message:
				struct ServerToClientMessage welcomeMessage;
				welcomeMessage.type = SYSTEM;
				sprintf(welcomeMessage.content,
						(clientConnections.clientCount > 1) ?
						"Welcome to the server. There are %d players connected." :
						"Welcome to the server. There is %d player connected.",
						clientConnections.clientCount);

				// Send the welcome message:
				gnutls_record_send(*tlsSession, &welcomeMessage, sizeof(struct ServerToClientMessage));
				
				// Report the new connection on the server:
				printf("New connection established. %d client(s), session ID %u.\n",
					   clientConnections.clientCount, tlsSession);
			}
			else
			{
				// Find the corresponding TLS session:
				struct ClientConnection * connection = findConnectionByFileDescriptor(&clientConnections, events[index].data.fd);
				if (connection != NULL)
				{
					// Read the data from the TLS session:
					struct ClientToServerMessage message;
					int returnValue = gnutls_record_recv(*connection->tlsSession, &message, sizeof(struct ClientToServerMessage));
				       
					if (returnValue == 0 || returnValue == -10)
					{
						epoll_ctl(connectedClients, EPOLL_CTL_DEL, connection->fileDescriptor, &watchedEvents);
						shutdown(connection->fileDescriptor, 2);
						close(connection->fileDescriptor);
						removeFromPlayerList(connection->player, globalPlayerList);
						deallocatePlayer(&connection->player);
						removeConnectionByFileDescriptor(&clientConnections, connection->fileDescriptor);					   
						continue;
					}
					else if (returnValue == sizeof(struct ClientToServerMessage))
					{
						// TODO: BEGIN ACTUAL COMMAND INTERPRETER
						// ONLY FOR DEMO
						if (strncmp(message.content, "NAME ", 5) == 0 && message.content[5] != '\0')
						{
							strncpy(connection->player->name, &message.content[5], 64);
							continue;
						}
						// ONLY FOR DEMO
]					   
						pushOutputMessage(globalOutputQueue, false, globalPlayerList, LOCAL_CHAT,
										  connection->player->name, message.content,
										  MESSAGE_NAME_LENGTH,  MESSAGE_CONTENT_LENGTH);						
					}
				}
				else
				{
					printf("Didn't find associated TLS Session!\n");
					fflush(stdout);
					// Remove the file descriptor from our watched set and close it:
					epoll_ctl(connectedClients, EPOLL_CTL_DEL, events[index].data.fd, &watchedEvents);
					close(events[index].data.fd);
					removeConnectionByFileDescriptor(&clientConnections, events[index].data.fd);
				}
			} 
		}

		// Output the queue:
		struct OutputMessage * currentMessage = NULL;
		while (globalOutputQueue->count != 0)
		{
			currentMessage = popOutputMessage(globalOutputQueue);
			struct PlayerListNode * currentPlayerNode = currentMessage->recepients->head;

			while (currentPlayerNode != NULL)
			{
				gnutls_record_send(*currentPlayerNode->player->connection->tlsSession,
								   currentMessage->message, sizeof(struct ServerToClientMessage));
				currentPlayerNode = currentPlayerNode->next;
			}

			if (currentMessage->deallocatePlayerList == true)
			{
				deallocatePlayerList(&currentMessage->recepients);
			}
			
			deallocateOutputMessage(&currentMessage);
		}
	}
	
	// Wait for all other threads to terminate:
	//pthread_join(schemeREPLThread, NULL);

	// Deallocate the global state structures:
	deallocatePlayerList(&globalPlayerList);
	
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
