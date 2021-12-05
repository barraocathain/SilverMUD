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
#include "misc/lists.h"
#include "misc/playerdata.h"
#include "misc/texteffects.h"
#include "misc/inputhandling.h"
const int PORT = 5000;
typedef struct sockaddr sockaddr;
	
int main()
{ 
	int socketFileDesc, connectionFileDesc, length, clientsAmount,
		socketCheck, activityCheck, readLength;
	int clientSockets[64];
	int maxClients = 64;
	userMessage messageBuffer;
	char receiveBuffer[2048];
	fd_set connectedClients;
	playerInfo connectedPlayers[64];
	struct sockaddr_in serverAddress, clientAddress;

	// Initialize areas:
	areaNode * areas = createAreaList(createArea("Spawn - North", "A large area, mostly empty, as if the designer hadn't bothered to put anything in it, just yet."));
	addAreaNodeToList(areas, createArea("Spawn - South", "A strange, white void. You feel rather uncomfortable."));
	addAreaNodeToList(areas, createArea("Temple of Emacs", "A beautifully ornate statue of GNU is above you on a pedestal. Inscribed into the pillar, over and over, is the phrase \"M-x exalt\", in delicate gold letters. You can't help but be awestruck."));
	createPath(getAreaFromList(areas, 0), getAreaFromList(areas, 1), "To South Spawn", "To North Spawn");
  	createPath(getAreaFromList(areas, 2), getAreaFromList(areas, 1), "Back to South Spawn", "Path to Enlightenment.");
	
	// Initialize playerdata:
	for (int index = 0; index < maxClients; index++) 
	{
		strcpy(connectedPlayers[index].playerName, "UNNAMED");
		connectedPlayers[index].currentArea = getAreaFromList(areas, 0);
	}
			
	// Give an intro: Display the Silverkin Industries logo and splash text.
	slowPrint(logostring, 3000);
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK SERVER ====--\nVersion Alpha 0.3\n", 5000);
	 
	// Initialize the sockets to 0, so we don't crash.
	for (int index = 0; index < maxClients; index++)  
	{  
		clientSockets[index] = 0;  
	}
	
	// Get a socket and make sure we actually get one.
	socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFileDesc == -1)
	{
		perror("Socket creation is \033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}

	else
	{
		slowPrint(" Socket creation is \033[32;40mGREEN.\033[0m\n", 5000);
	}

	bzero(&serverAddress, sizeof(serverAddress));
  
	// Assign IP and port:
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);
  
	// Binding newly created socket to given IP, and checking it works:
	if ((bind(socketFileDesc, (sockaddr*)&serverAddress, sizeof(serverAddress))) != 0)
	{
		perror("Socket binding is \033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}
	else
	{
		slowPrint(" Socket binding is \033[32;40mGREEN.\033[0m\n", 5000);
	}
	
	// Let's start listening:
	if ((listen(socketFileDesc, 64)) != 0)
	{
		perror("Server listening is \033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}
	else		
	{
		slowPrint(" Server listening is \033[32;40mGREEN.\033[0m\n", 5000);
	}
	length = sizeof(clientAddress);

	// Accept the data packet from client and verification
	while (1)
	{
		FD_ZERO(&connectedClients);
		FD_SET(socketFileDesc, &connectedClients);
		clientsAmount = socketFileDesc;
		bzero(receiveBuffer, sizeof(receiveBuffer));
		for (int i = 0; i < maxClients; i++)  
		{  
			// Just get the one we're working with to another name:
			socketCheck = clientSockets[i];  
                 
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
			if ((connectionFileDesc = accept(socketFileDesc, 
											 (struct sockaddr *)&clientAddress, (socklen_t*)&length))<0)  
			{  
				perror("Failed to accept connection. Aborting.\n");  
				exit(EXIT_FAILURE);  
			}  
             
			// Print new connection details:
			printf("Client connected: Socket file descriptor: #%d, IP address: %s, Port: %d.\n",
				   connectionFileDesc, inet_ntoa(clientAddress.sin_addr) , ntohs
				   (clientAddress.sin_port));  
           
			// See if we can put in the client:
			for (int i = 0; i < maxClients; i++)  
			{  
				// When there is an empty slot, pop it in:
				if( clientSockets[i] == 0 )  
				{  
					clientSockets[i] = connectionFileDesc;  
					printf("Adding to list of sockets as %d.\n" , i);                           
					break;  
				}  
			}  
		}
		else
		{
			// Otherwise, it's a client socket to be interacted with:
			for (int i = 0; i < maxClients; i++)  
			{  
				socketCheck = clientSockets[i];  
                 
				if (FD_ISSET(socketCheck, &connectedClients))  
				{  
					//Check if it was for closing, and also read the incoming message
					explicit_bzero(receiveBuffer, sizeof(receiveBuffer));
					readLength = read(socketCheck, receiveBuffer, sizeof(receiveBuffer));
					userInputSanatize(receiveBuffer, 2048);
					if (readLength == 0)
					{
						// Somebody disconnected , get his details and print:
						getpeername(socketCheck, (struct sockaddr*)&clientAddress, (socklen_t*)&length);  
						printf("Client disconnected: IP Address: %s, Port: %d.\n", 
							   inet_ntoa(clientAddress.sin_addr) , ntohs(clientAddress.sin_port));  
						
						// Close the socket and mark as 0 in list for reuse:
						close(socketCheck);  
						clientSockets[i] = 0;  	
				}
					// Name change command: Move logic to a command interpreter later:
					else if (receiveBuffer[0] == '/')
					{					
						if(strncmp(receiveBuffer, "/NAME", 5) == 0)
						{
							char newName[32];
							strncpy(newName, &receiveBuffer[6], 32);
							// Remove newlines:
							for (int index = 0; index < 32; index++)
							{
								if (newName[index] == '\n')
								{
									newName[index] = '\0';
								}
							}
							for (int index = 0; index < maxClients; index++)
							{
								if(strncmp(newName, connectedPlayers[index].playerName, 32) == 0)
								{
									break;
								}
							}
							if(newName[0] != '\0')
							{
								strncpy(connectedPlayers[i].playerName, newName, 32);
							}
						}
						else if(strncmp(receiveBuffer, "/LOOK", 5) == 0)
						{
							strcat(messageBuffer.messageContent, connectedPlayers[i].currentArea->areaDescription);
							strcat(messageBuffer.messageContent, "\nYou see:");		
							for(int index = 0; index < 16; index++)
							{								
								if(connectedPlayers[i].currentArea->areaExits[index] != NULL)
								{
									strcat(messageBuffer.messageContent, "\n - ");
									strcat(messageBuffer.messageContent, connectedPlayers[i].currentArea->areaExits[index]->pathName);
								}
							}
							write(socketCheck, messageBuffer.senderName, sizeof(messageBuffer.senderName));
							write(socketCheck, messageBuffer.messageContent, sizeof(messageBuffer.messageContent));
							bzero(messageBuffer.senderName, sizeof(messageBuffer.senderName));
							bzero(messageBuffer.messageContent, sizeof(messageBuffer.messageContent));
						}
						else if(strncmp(receiveBuffer, "/MOVE", 5) == 0)
						{
							char requestedPath[32];
							strncpy(requestedPath, &receiveBuffer[6], 32);
							userInputSanatize(requestedPath, 32);
							// Remove newlines:
							for (int index = 0; index < 32; index++)
							{
								if (requestedPath[index] == '\n')
								{
									requestedPath[index] = '\0';
								}
							}
							requestedPath[31] = '\0';
							if(movePlayerToArea(&connectedPlayers[i], requestedPath) == 0)
							{
								strcpy(messageBuffer.senderName, "\0");
								strcpy(messageBuffer.messageContent, connectedPlayers[i].currentArea->areaDescription);
								write(socketCheck, messageBuffer.senderName, sizeof(messageBuffer.senderName));
								write(socketCheck, messageBuffer.messageContent, sizeof(messageBuffer.messageContent));									  
							}
							else
							{
								strcpy(messageBuffer.senderName, "");
								strcpy(messageBuffer.messageContent, "You can't go somewhere that doesn't exist!");
								write(socketCheck, messageBuffer.senderName, sizeof(messageBuffer.senderName));
								write(socketCheck, messageBuffer.messageContent, sizeof(messageBuffer.messageContent));
								bzero(messageBuffer.senderName, sizeof(messageBuffer.senderName));
								bzero(messageBuffer.messageContent, sizeof(messageBuffer.messageContent));
							}
						}
					}
					// Echo back the message that came in:
					else if (receiveBuffer[0] == '\n')
					{
						continue;
					}
					else 
					{  
						printf("%d/%s/%s: %s", clientSockets[i], connectedPlayers[i].currentArea->areaName, connectedPlayers[i].playerName, receiveBuffer);
						fflush(stdout);
						strcpy(messageBuffer.senderName, connectedPlayers[i].playerName);
						strcpy(messageBuffer.messageContent, receiveBuffer);
						for (int sendIndex = 0; sendIndex < clientsAmount; sendIndex++)
						{
							if(clientSockets[sendIndex] != STDIN_FILENO && (connectedPlayers[i].currentArea == connectedPlayers[sendIndex].currentArea))
							{
								write(clientSockets[sendIndex], messageBuffer.senderName, sizeof(messageBuffer.senderName));
								write(clientSockets[sendIndex], messageBuffer.messageContent, sizeof(messageBuffer.messageContent));
							}
						}
						bzero(messageBuffer.senderName, sizeof(messageBuffer.senderName));
						bzero(messageBuffer.messageContent, sizeof(messageBuffer.messageContent));
					}  
				}
			}
		}
	}
}
