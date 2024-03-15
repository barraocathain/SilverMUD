// =========================================
// | SilverMUD Client - main.c             |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <ncurses.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <gnutls/gnutls.h>

#include "../config.h"
#include "../messages.h"
#include "client-drawing.h"
#include "receiving-thread.h"

int main (int argc, char ** argv)
{
	static char serverPort[HOST_NAME_MAX] = "5050";
	static char serverHostname[HOST_NAME_MAX] = "127.0.0.1";
	struct addrinfo * serverInformation;
	
	// Print a welcome message:
	printf("SilverMUD Client - Starting Now.\n"
		   "================================\n");

	// Configure command-line options:
	static struct option longOptions[] =
	{
		{"host", required_argument, 0, 'h' },
		{"port", required_argument, 0, 'p' }
	};
	bool hostSpecified = false, portSpecified = false;
	
	// Parse command-line options:
	int selectedOption = 0, optionIndex = 0;
	
	while ((selectedOption = getopt_long(argc, argv, "h:p:", longOptions, &optionIndex)) != -1) 
	{
		switch (selectedOption)
		{
			case 'h':
			{
				printf("Connecting to host: %s\n", optarg);
				hostSpecified = true;
				strncpy(serverHostname, optarg, HOST_NAME_MAX);
				break;
			}
			case 'p':
			{
				printf("Connecting to port: %s\n", optarg);
				portSpecified = true;
				strncpy(serverPort, optarg, HOST_NAME_MAX);
				break;
			}
		}
	}
	
	// Create a socket for communicating with the server:
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		printf("Socket creation failed. Aborting.\n");
		exit(EXIT_FAILURE);
	}

	// Set up the server address structure to point to the server:
	if (getaddrinfo(serverHostname, serverPort, NULL, &serverInformation) != 0)
	{
		printf("Server lookup failed. Aborting.\n");
		exit(EXIT_FAILURE);
	}
	
	// Connect to the server:
	if (connect(serverSocket, serverInformation->ai_addr, serverInformation->ai_addrlen) != 0)
	{
		printf("Failed to connect to the server. Aborting.\n");
		exit(EXIT_FAILURE);
	}

	// Set up a GnuTLS session and handshake with the server:
	gnutls_session_t tlsSession = NULL;
	if (gnutls_init(&tlsSession,  GNUTLS_CLIENT) < 0)
	{	
		exit(EXIT_FAILURE);
	}

	gnutls_anon_client_credentials_t clientKey = NULL;
	gnutls_anon_allocate_client_credentials(&clientKey);
	gnutls_credentials_set(tlsSession, GNUTLS_CRD_ANON, &clientKey);

	gnutls_transport_set_int(tlsSession, serverSocket);
	gnutls_priority_set_direct(tlsSession, "PERFORMANCE:+ANON-ECDH:+ANON-DH", NULL);

	gnutls_handshake_set_timeout(tlsSession, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

	int returnValue = -1;
	do
	{
		returnValue = gnutls_handshake(tlsSession);
	}
	while (returnValue < 0 && gnutls_error_is_fatal(returnValue) == 0);

	// Initialize ncurses:
	initscr();
	keypad(stdscr, TRUE);
	
	if (!has_colors())
	{
		endwin();
		exit(EXIT_FAILURE);
	}

	// Enable colours:
	start_color();
	use_default_colors();
	init_pair(1, COLOR_GREEN,     -1);
	init_pair(2, COLOR_YELLOW,    -1);
	init_pair(3, COLOR_RED,       -1);
	init_pair(4, COLOR_BLUE,      -1);
	init_pair(5, COLOR_CYAN,      -1);
	init_pair(6, COLOR_MAGENTA,   -1);
	
	// Variables needed for the main loop:
	int height, width;
	getmaxyx(stdscr, height, width);
	struct ClientToServerMessage message;
	
	WINDOW * chatWindow, * gameWindow, * inputWindow;
	inputWindow = newwin(1, width - 2, height - 1, 1);
	gameWindow = newwin((height / 2) - 1, width - 2, 1, 1);
	chatWindow = newwin((height / 2) - 3, width - 2, (height / 2) + 1, 1);

	scrollok(gameWindow, TRUE);
	scrollok(chatWindow, TRUE);
	scrollok(inputWindow, TRUE);
	
	redrawClientLayout(gameWindow, chatWindow, inputWindow);
	
	struct ReceivingThreadArguments receivingThreadArguments;
	receivingThreadArguments.chatWindow = chatWindow;
	receivingThreadArguments.gameWindow = gameWindow;
	receivingThreadArguments.inputWindow = inputWindow;
	receivingThreadArguments.session = tlsSession;

	pthread_t receivingThread;
	pthread_create(&receivingThread, NULL, receivingThreadHandler, &receivingThreadArguments);
	
	while (true)
	{				
		wgetnstr(inputWindow, message.content, 1024);

		if (message.content[0] != '\0')
		{
			gnutls_record_send(tlsSession, &message, 1024);			
		}
	}
	
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
