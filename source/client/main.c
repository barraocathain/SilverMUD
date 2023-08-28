// =========================================
// | SilverMUD Client - main.c             |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <ncurses.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <gnutls/gnutls.h>

#include "../config.h"
#include "../messages.h"

int main (int argc, char ** argv)
{	
	// Print a welcome message:
	printf("SilverMUD Client - Starting Now.\n"
		   "================================\n");

	// Create a socket for communicating with the server:
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		printf("Socket creation failed. Aborting.\n");
		exit(EXIT_FAILURE);
	}

	// Set up the server address structure to point to the server:
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(5000);	

	// Connect to the server:
	if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) != 0)
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
	init_pair(1, COLOR_GREEN, -1);

	// Variables needed for the main loop:
	int height, width;
	getmaxyx(stdscr, height, width);
	struct ClientToServerMessage message;
	
	WINDOW * chatWindow, * gameWindow;	
	gameWindow = newwin((height / 2) - 1, width - 2, 1, 1);
	chatWindow = newwin((height / 2) - 3, width - 2, (height / 2) + 1, 1);

	scrollok(gameWindow, TRUE);
	scrollok(chatWindow, TRUE);
	
	while (true)
	{		
		// Store the current size of the terminal:
		getmaxyx(stdscr, height, width);		

		
		// Draw the lines that will seperate windows:
		wborder(stdscr, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		mvwhline(stdscr, 0, 0, '=', width);
		mvwhline(stdscr, height / 2, 0, '=', width);
		mvwhline(stdscr, height - 2, 0, '=', width);

		// Write the labels for windows:
		attron(COLOR_PAIR(1));
		mvwprintw(stdscr, 0, 1, " SilverMUD | Version %s ", PACKAGE_VERSION);
		mvwprintw(stdscr, height / 2, 1, " Chat ");
		mvwprintw(stdscr, height - 2, 1, " Input ");
		attroff(COLOR_PAIR(1));

		wrefresh(stdscr);
		
		// Move the windows into place:
		mvwin(gameWindow, 1, 1);
		mvwin(chatWindow, (height / 2) + 1 , 1);
		wresize(gameWindow, (height - 2) / 2, width - 2);
		wresize(chatWindow, ((height - 3) / 2) - (1 - (height % 2)), width - 2);

		wrefresh(gameWindow);
		wrefresh(chatWindow);		
		
		// Move to the input area:
		wmove(stdscr, height - 1, 1);
		wgetnstr(stdscr, message.content, 1024);

		// Clear the input area:
		wmove(stdscr, height - 2, 1);
		clrtoeol();
		
		if (message.content[0] != '\0')
		{
			wprintw(gameWindow, "\n%s", message.content);
			wprintw(chatWindow, "\n%s", message.content);
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
