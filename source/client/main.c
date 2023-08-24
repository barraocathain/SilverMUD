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


	char buffer[2048];
	while (true)
	{
		fgets(buffer, 2048, stdin);
		gnutls_record_send(tlsSession, &buffer, 2048);
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
