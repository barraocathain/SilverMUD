// Silverkin Industries Comm-Link Server, Engineering Sample Alpha 0.3.
// PROJECT CODENAME: WHAT DO I PAY YOU FOR? | Level-3 Clearance.
// Barry Kane, 2021
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <gnutls/gnutls.h>
#include "misc/lists.h"
#include "misc/playerdata.h"
#include "misc/texteffects.h"
#include "misc/inputoutput.h"
#include "misc/inputhandling.h"

const int PORT = 5000;
const int PLAYERCOUNT = 64;
typedef struct sockaddr sockaddr;
	
int main()
{
	bool keepRunning = true;
	int socketFileDesc, connectionFileDesc, length, clientsAmount,
		socketCheck, activityCheck, readLength, returnVal;
	fd_set connectedClients;
	int clientSockets[PLAYERCOUNT];
	userMessage sendBuffer, receiveBuffer;
	playerInfo connectedPlayers[PLAYERCOUNT];
	char testString[32] = "Hehe.";
	struct sockaddr_in serverAddress, clientAddress;
	
	// Initialize playerdata:
	for (int index = 0; index < PLAYERCOUNT; index++) 
	{
		strcpy(connectedPlayers[index].playerName, "UNNAMED");
	}
			
	// Give an intro: Display the Silverkin Industries logo and splash text.
	slowPrint(logostring, 3000);
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK SERVER ====--\nVersion Alpha 0.3\n", 5000);
	 
	// Initialize the sockets to 0, so we don't crash.
	for (int index = 0; index < PLAYERCOUNT; index++)  
	{  
		clientSockets[index] = 0;  
	}
	
	// Get a socket and make sure we actually get one.
	socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFileDesc == -1)
	{
		perror("\tSocket Creation is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}

	else
	{
		slowPrint("\tSocket Creation is:\t\033[32;40mGREEN.\033[0m\n", 5000);
	}

	bzero(&serverAddress, sizeof(serverAddress));
  
	// Assign IP and port:
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);
  
	// Binding newly created socket to given IP, and checking it works:
	if ((bind(socketFileDesc, (sockaddr*)&serverAddress, sizeof(serverAddress))) != 0)
	{
		perror("\tSocket Binding is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}
	
	else
	{
		slowPrint("\tSocket Binding is:\t\033[32;40mGREEN.\033[0m\n", 5000);
	}
	
	// Let's start listening:
	if ((listen(socketFileDesc, PLAYERCOUNT)) != 0)
	{
		perror("\tServer Listening is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(EXIT_FAILURE);
	}
	else		
	{
		slowPrint("\tServer Listening is:\t\033[32;40mGREEN.\033[0m\n", 5000);
	}
	length = sizeof(clientAddress);

	gnutls_session_t tlssessions[PLAYERCOUNT];
	gnutls_anon_server_credentials_t serverkey = NULL;
	gnutls_anon_allocate_server_credentials(&serverkey);
	gnutls_anon_set_server_known_dh_params(serverkey, GNUTLS_SEC_PARAM_MEDIUM);
	
	// Initialize all the TLS Sessions to NULL: We use this to check if it's an "empty connection."
	for (int index = 0; index < PLAYERCOUNT; index++)  
	{  
		tlssessions[index] = NULL;
		if (gnutls_init(&tlssessions[index], GNUTLS_SERVER) < 0)
		{
			perror("\tTLS Sessions Initialization is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
			exit(EXIT_FAILURE);
		}
		gnutls_priority_set_direct(tlssessions[index], "NORMAL:+ANON-ECDH:+ANON-DH", NULL);
		gnutls_credentials_set(tlssessions[index], GNUTLS_CRD_ANON, &serverkey);
		gnutls_handshake_set_timeout(tlssessions[index], GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
	}
	slowPrint("\tTLS Sessions Initialization is:\t\033[32;40mGREEN.\033[0m\n", 5000);

	while(keepRunning)
	{
		// Clear the set of file descriptors and add the master socket:
		FD_ZERO(&connectedClients);
		FD_SET(socketFileDesc, &connectedClients);
		clientsAmount = socketFileDesc;

		// Find all sockets that are still working and place them in the set:
		for(int index = 0; index < PLAYERCOUNT; index++)
		{
			// Just get the one we're working with to another name:
			socketCheck = clientSockets[index];  
                 
			// If it's working, bang it into the list:
			if(socketCheck > 0)
			{
				FD_SET(socketCheck, &connectedClients);  
			}
			// The amount of clients is needed for select():
			if(socketCheck > clientsAmount)
			{
				clientsAmount = socketCheck;
			}
		}

		// See if a connection is ready to be interacted with:
		activityCheck = select((clientsAmount + 1), &connectedClients, NULL, NULL, NULL);

		// Check if select() worked:
		if ((activityCheck < 0) && (errno != EINTR))  
		{ 
			perror("Error in select(), retrying.\n");  
		}

		// If it's the master socket selected, there is a new connection:
		if (FD_ISSET(socketFileDesc, &connectedClients))  
		{  
			if ((connectionFileDesc = accept(socketFileDesc, (struct sockaddr *)&clientAddress, (socklen_t*)&length)) < 0)  
			{  
				perror("Failed to accept connection. Aborting.\n");  
				exit(EXIT_FAILURE);  
			} 
			// See if we can put in the client:
			for (int index = 0; index < PLAYERCOUNT; index++)  
			{  
				// When there is an empty slot, pop it in:
				if (clientSockets[index] == 0)  
				{  
					clientSockets[index] = connectionFileDesc;  
					printf("Adding to list of sockets as %d.\n", index);
					gnutls_transport_set_int(tlssessions[index], clientSockets[index]);
					do 
					{
						returnVal = gnutls_handshake(tlssessions[index]);
					}
					while (returnVal < 0 && gnutls_error_is_fatal(returnVal) == 0);
					strcpy(sendBuffer.senderName, "");
					strcpy(sendBuffer.messageContent, "Welcome to the server!");
					messageSend(tlssessions[index], &sendBuffer);
					break;  
				}  
			}  
		}
		// Otherwise, it's a client we need to interact with:
		else
		{
			for (int index = 0; index < PLAYERCOUNT; index++)  
			{  
				socketCheck = clientSockets[index];

				if(FD_ISSET(socketCheck, &connectedClients))
				{
					messageReceive(tlssessions[index], &receiveBuffer);
					sprintf(testString, "User %d", index);
					strcpy(sendBuffer.senderName, testString);
					userInputSanatize(receiveBuffer.messageContent, sizeof(receiveBuffer.messageContent));
					strcpy(sendBuffer.messageContent, receiveBuffer.messageContent);
					for (int sendIndex = 0; sendIndex < clientsAmount; sendIndex++)
					{
						messageSend(tlssessions[sendIndex], &sendBuffer);
					}
				}			
			}
		}        
	}
	return 0;
}

