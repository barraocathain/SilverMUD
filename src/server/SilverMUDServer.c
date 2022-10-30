// Silverkin Industries Comm-Link Server, Engineering Sample Alpha 0.3.
// PROJECT CODENAME: WHAT DO I PAY YOU FOR? | Level-3 Clearance.
// Barry Kane, 2021
#include <time.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <gnutls/gnutls.h>

#include "../areadata.h"
#include "../gamelogic.h"
#include "../constants.h"
#include "../playerdata.h"
#include "../texteffects.h"
#include "../inputoutput.h"

typedef struct sockaddr sockaddr;
void sigintHandler(int signal)
{
	exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv)
{
	time_t currentTime;
	int socketFileDesc, connectionFileDesc, length, clientsAmount,
		socketCheck, activityCheck, returnVal;
	fd_set connectedClients;
	pthread_t gameLogicThread;
	int clientSockets[PLAYERCOUNT];
	userMessage sendBuffer, receiveBuffer;
	playerInfo connectedPlayers[PLAYERCOUNT];
	char testString[32] = "Hehe.";
	struct sockaddr_in serverAddress, clientAddress;
	inputMessageQueue * inputQueue = createInputMessageQueue();
	outputMessageQueue * outputQueue = createOutputMessageQueue();
	
	// Set the handler for SIGINT:
	signal(2, sigintHandler);
	
	// -==[ TEST GAME-STATE INITIALIZATION ]==-
	// Initialize test areas:
	areaNode * areas = createAreaList(createArea("Login Area", "Please login with the /join command."));
	addAreaNodeToList(areas, createArea("Temple Entrance",
										"You are standing outside a large, elaborate temple, of white marble and delicate construction. "
										"Etched onto the left pillar next to the large opening is the same symbol, over and over again, a gentle curve with it's ends pointing to the right. "
										"A similar symbol is on the right pillar, but it's ends are pointing to the left. "));
	
	addAreaNodeToList(areas, createArea("The Hall of Documentation",
										"Just past the threshold of the entrance lies a large hall, with bookshelves lining the walls, ceiling to floor. "
										"The shelves are filled to the brim with finely-bound books, each with titles in silver lettering on the spine. "
										"There are countless books, but you notice a large lectern in the center of the room, and a path leading upwards at the back. "));
	
	addAreaNodeToList(areas, createArea("Monument to GNU",
										"A beautifully ornate statue of GNU is above you on a pedestal. "
										"Inscribed into the pillar, over and over, is the phrase \"M-x exalt\", in delicate gold letters. "
										"You can't help but be awestruck."));
	// Initialize test paths:
	createPath(getAreaFromList(areas, 1), getAreaFromList(areas, 2), "Go inside the temple.", "Leave the temple.");
  	createPath(getAreaFromList(areas, 3), getAreaFromList(areas, 2), "Back to the Hall of Documentation.", "Path to Enlightenment.");
	skillList * globalSkillList = malloc(sizeof(skillList));
	globalSkillList->head = NULL;

	// Create a few basic skills:
	createSkill(globalSkillList, "Medicine", 8, true);
	createSkill(globalSkillList, "Lockpicking", 12, true);
	createSkill(globalSkillList, "Programming", 12, true);
	createSkill(globalSkillList, "Sensor Reading", 14, false);
	createSkill(globalSkillList, "Starship Piloting", 17, true);
	createSkill(globalSkillList, "Mechanical Repair", 17, true);
	
	// Initialize playerdata:
	for (int index = 0; index < PLAYERCOUNT; index++) 
	{
		sprintf(testString, "UNNAMED %d", index);
		// OH NO IT'S NOT MEMORY SAFE BETTER REWRITE IT IN RUST
		// But wait, we know the string won't be too big, so it's fine.
		strcpy(connectedPlayers[index].playerName, testString);
		connectedPlayers[index].currentArea = getAreaFromList(areas, 0);
		connectedPlayers[index].stats = calloc(1, sizeof(statBlock));
		connectedPlayers[index].stats->specPoints = 30;
		connectedPlayers[index].stats->skillPoints = 30;
		connectedPlayers[index].skills = calloc(1, sizeof(skillList));
		connectedPlayers[index].skills->head = NULL;
	}

	// -==[ TEST GAME-STATE INITIALIZATION END ]==-
	
	// Give an intro: Display the Silverkin Industries logo and splash text.
	slowPrint(logostring, 3000);
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK SERVER ====--\nVersion Alpha 0.3\n", 5000);

	// Seed random number generator from the current time:
	srandom((unsigned)time(&currentTime));
	
	// Initialize the sockets to 0, so we don't crash.
	for (int index = 0; index < PLAYERCOUNT; index++)  
	{  
		clientSockets[index] = 0;  
	}
	
	// Get a socket and make sure we actually get one.
	socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFileDesc == -1)
	{
		fprintf(stderr, "\tSocket Creation is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}

	else
	{
		slowPrint("\tSocket Creation is:\t\033[32;40mGREEN.\033[0m\n", 5000);
	}

	// 
	bzero(&serverAddress, sizeof(serverAddress));
  
	// Assign IP and port:
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);
  
	// Binding newly created socket to given IP, and checking it works:
	if ((bind(socketFileDesc, (sockaddr*)&serverAddress, sizeof(serverAddress))) != 0)
	{
		fprintf(stderr, "\tSocket Binding is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}
	
	else
	{
		slowPrint("\tSocket Binding is:\t\033[32;40mGREEN.\033[0m\n", 5000);
	}
	
	// Let's start listening:
	if ((listen(socketFileDesc, PLAYERCOUNT)) != 0)
	{
		fprintf(stderr, "\tServer Listener is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(EXIT_FAILURE);
	}
	else		
	{
		slowPrint("\tServer Listener is:\t\033[32;40mGREEN.\033[0m\n", 5000);
	}
	length = sizeof(clientAddress);

	// Declare the needed variables for TLS sessions:
	gnutls_session_t tlssessions[PLAYERCOUNT];
	gnutls_anon_server_credentials_t serverkey = NULL;
	gnutls_anon_allocate_server_credentials(&serverkey);
	gnutls_anon_set_server_known_dh_params(serverkey, GNUTLS_SEC_PARAM_MEDIUM);
	
	// Initialize all the TLS sessions to NULL: We use this to check if it's an "empty connection."
	for (int index = 0; index < PLAYERCOUNT; index++)  
	{  
		tlssessions[index] = NULL;
		if (gnutls_init(&tlssessions[index], GNUTLS_SERVER) < 0)
		{
			fprintf(stderr, "\tTLS Sessions Initialization is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
			exit(EXIT_FAILURE);
		}
		gnutls_priority_set_direct(tlssessions[index], "NORMAL:+ANON-ECDH:+ANON-DH", NULL);
		gnutls_credentials_set(tlssessions[index], GNUTLS_CRD_ANON, &serverkey);
		gnutls_handshake_set_timeout(tlssessions[index], GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
  	}
	slowPrint("\tTLS Preparation is:\t\033[32;40mGREEN.\033[0m\n", 5000);

	// Prepare the game logic thread:
	gameLogicParameters * gameLogicThreadParameters = malloc(sizeof(gameLogicParameters));
	gameLogicThreadParameters->connectedPlayers = connectedPlayers;
	gameLogicThreadParameters->playerCount = &clientsAmount;
	gameLogicThreadParameters->globalSkillList = globalSkillList;
	gameLogicThreadParameters->outputQueue = outputQueue;
	gameLogicThreadParameters->inputQueue = inputQueue;
	gameLogicThreadParameters->areaList = areas;
	pthread_create(&gameLogicThread, NULL, &gameLogicLoop, gameLogicThreadParameters);

	slowPrint("\tEvent Thread is:\t\033[32;40mGREEN.\033[0m\n", 5000);
	slowPrint("=====\n", 5000); 
	struct timeval timeout = {0, 500};
	
	while(true)
	{
		// Clear the set of file descriptors angad add the master socket:
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
		activityCheck = select((clientsAmount + 1), &connectedClients, NULL, NULL, &timeout);

		// Check if select() worked:
		if ((activityCheck < 0) && (errno != EINTR))  
		{ 
			fprintf(stderr, "Error in select(), retrying.\n");  
		}

		// If it's the master socket selected, there is a new connection:
		if (FD_ISSET(socketFileDesc, &connectedClients))  
		{  
			if ((connectionFileDesc = accept(socketFileDesc, (struct sockaddr *)&clientAddress, (socklen_t*)&length)) < 0)  
			{  
				fprintf(stderr, "Failed to accept connection. Aborting.\n");  
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
					
					// Send a greeting message:
					strcpy(sendBuffer.senderName, "");
					strcpy(sendBuffer.messageContent, "Welcome to the server!");
					messageSend(tlssessions[index], &sendBuffer);
					strcpy(receiveBuffer.messageContent, "/look");
					queueInputMessage(inputQueue, receiveBuffer, &connectedPlayers[index]);
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
					int returnVal = messageReceive(tlssessions[index], &receiveBuffer);
					// If player has disconnected:
					if(returnVal == -10 || returnVal == 0)
					{
						// Close the session:
						gnutls_bye(tlssessions[index], GNUTLS_SHUT_WR);
						gnutls_deinit(tlssessions[index]);
						shutdown(clientSockets[index], 2);
						close(clientSockets[index]);
						clientSockets[index] = 0;
						tlssessions[index] = NULL;

						// Clear out the old player state so that a new one may join:
						sprintf(testString, "UNNAMED %d", index);
						strcpy(connectedPlayers[index].playerName, testString);
						connectedPlayers[index].currentArea = getAreaFromList(areas, 0);

						// Prepare a fresh SSL session for the next new player:
						gnutls_init(&tlssessions[index], GNUTLS_SERVER);
						gnutls_priority_set_direct(tlssessions[index], "NORMAL:+ANON-ECDH:+ANON-DH", NULL);
						gnutls_credentials_set(tlssessions[index], GNUTLS_CRD_ANON, &serverkey);
						gnutls_handshake_set_timeout(tlssessions[index], GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
					}
					// Otherwise, they've sent a message:
					else 
					{
						queueInputMessage(inputQueue, receiveBuffer, &connectedPlayers[index]);
					}
				}			
			}
		}

		// Run through the output queue and send all unused messages:
		while(outputQueue->currentLength != 0)
		{
			while(outputQueue->lock);
			outputQueue->lock = true;
			outputMessage * message = peekOutputMessage(outputQueue);
			outputQueue->lock = false;
			
			// If the first target is set to NULL, it's intended for all connected:
			if(message->targets[0] == NULL)
			{
				for (int index = 0; index < PLAYERCOUNT; index++)  
				{
					messageSend(tlssessions[index], message->content);
				}
			}
			else
			{
				int targetIndex = 0;
				for(int index = 0; index < PLAYERCOUNT; index++)
				{
					if(message->targets[targetIndex] == NULL)
					{
						break;
					}
					if(&connectedPlayers[index] == message->targets[targetIndex])
					{
						targetIndex++;
						messageSend(tlssessions[index], message->content);
					}
				}
			}
			dequeueOutputMessage(outputQueue);
		}
	}
	pthread_cancel(gameLogicThread);
	exit(EXIT_SUCCESS);
}

