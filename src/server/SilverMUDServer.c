// Silverkin Industries Comm-Link Server, Engineering Sample Alpha 0.5
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
#include <getopt.h>
#include <ncurses.h>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <gnutls/gnutls.h>

#include "../queue.h"
#include "../areadata.h"
#include "../gamelogic.h"
#include "../constants.h"
#include "../playerdata.h"
#include "../linkedlist.h"
#include "../texteffects.h"
#include "../inputoutput.h"
#include "../schemeintegration.h"

typedef struct sockaddr sockaddr;
void sigintHandler(int signal)
{
	printf("Caught signal %d.\n", signal);
	_exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv)
{
	time_t currentTime;
	unsigned delay = 800;
	int socketFileDesc, connectionFileDesc, length, clientsAmount,
		socketCheck, activityCheck, returnVal;
	fd_set connectedClients;
	pthread_t gameLogicThread, outputThread, schemeThread;
	int clientSockets[PLAYERCOUNT];
	userMessage sendBuffer, receiveBuffer;
	playerInfo connectedPlayers[PLAYERCOUNT];
	char testString[32] = "Hehe.";
	struct sockaddr_in serverAddress, clientAddress;
	char motd[2048] = "Please login with the /join command.";
	queue * inputQueue = createQueue(), * outputQueue = createQueue();
	
	// Parse command-line options:
	int currentopt = 0;
	while ((currentopt = getopt(argc, argv, "d:m:")) != -1)
	{
		switch(currentopt)
		{
			case 'd':
			{
				delay = atoi(optarg);
				break;
			}
			case 'm':
			{
				strncpy(motd, optarg, strlen(optarg) + 1);
				break;
			}
		}
	}
	
	// Set the handler for SIGINT:
	signal(2, sigintHandler);
	
	// -==[ TEST GAME-STATE INITIALIZATION ]==-
	// Initialize test areas:
	list * areas = createList(AREA);
	addToList(areas, createArea("Login Area", motd), AREA);

	// Create the areas:
	addToList(areas, createArea("Octal One - Docking Bay Alpha",
								"You are standing in the main docking bay of the largest station in the Octal System. "
								"The sheer size of the bay is awe-inpiring. The number of ships is endless. "
								"The bay is curved along with the body of the station. A catwalk runs along the back wall of the bay. "
								"Two large arches lie at each end, leading to the other bays, and in the center, a set of doors leading to the interior of the station."), AREA);

	addToList(areas, createArea("Octal One - Station Access Control",
								"You enter into the hallway leading to the main interior of the station."
								"The attendant informs you that due to a computer error, exits cannot be proccessed at the moment,"
								" so you will be unable to leave, until it is resolved. "
								"He apologizes profusely for the inconvenience, and clears you for entry if you wish to continue."), AREA);
		
	addToList(areas, createArea("Octal One - Floor Zero",
								"You've never quite seen so many people in one place. A large ring of shopfronts surrounds an area filled with benches and tables. "
								"There's so many buisnesses in sight that you feel you could find everything you need, and this is only one of 25 main floors, "
								"not to mention the 6 outer pylons which surround the main hull of the station. Staircases lead to an upper platform allowing access to the pylons. "), AREA);
					  
	// Initialize test paths:
	createPath(getFromList(areas, 1)->area, getFromList(areas, 2)->area,
			   "Enter the station interior.", "Return to Docking Bay Alpha.");
  	createOneWayPath(getFromList(areas, 2)->area, getFromList(areas, 3)->area,
					 "Continue to station interior. ");
	
	list * globalSkillList = createList(SKILL);

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
		connectedPlayers[index].currentArea = getFromList(areas, 0)->area;
		connectedPlayers[index].stats = calloc(1, sizeof(statBlock));
		connectedPlayers[index].stats->specPoints = 30;
		connectedPlayers[index].stats->skillPoints = 30;
		connectedPlayers[index].skills = createList(SKILL);
	}

	// -==[ TEST GAME-STATE INITIALIZATION END ]==-
	
	// Give an intro: Display the Silverkin Industries logo and splash text.
	slowPrint(logostring, delay);
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK SERVER ====--\nVersion Alpha 0.5\n", delay);

	// Seed random number generator from the current time:
	srand((unsigned)time(&currentTime));
	
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
		slowPrint("\tSocket Creation is:\t\033[32;40mGREEN.\033[0m\n", delay);
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
  
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
		slowPrint("\tSocket Binding is:\t\033[32;40mGREEN.\033[0m\n", delay);
	}
	
	// Let's start listening:
	if ((listen(socketFileDesc, PLAYERCOUNT)) != 0)
	{
		fprintf(stderr, "\tServer Listener is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(EXIT_FAILURE);
	}
	else		
	{
		slowPrint("\tServer Listener is:\t\033[32;40mGREEN.\033[0m\n", delay);
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
	slowPrint("\tTLS Preparation is:\t\033[32;40mGREEN.\033[0m\n", delay);

	// Prepare the game logic thread:
	gameLogicParameters * gameLogicThreadParameters = malloc(sizeof(gameLogicParameters));
	gameLogicThreadParameters->connectedPlayers = connectedPlayers;
	gameLogicThreadParameters->playerCount = &clientsAmount;
	gameLogicThreadParameters->globalSkillList = globalSkillList;
	gameLogicThreadParameters->outputQueue = outputQueue;
	gameLogicThreadParameters->inputQueue = inputQueue;
	gameLogicThreadParameters->areaList = areas;
	pthread_create(&gameLogicThread, NULL, &gameLogicHandler, gameLogicThreadParameters);

	slowPrint("\tEvent Thread is:\t\033[32;40mGREEN.\033[0m\n", delay);
	
	// Prepare the output queue thread:
	outputThreadParameters * outputParameters = malloc(sizeof(outputThreadParameters));
	outputParameters->outputQueue = outputQueue;
	outputParameters->tlssessions = tlssessions;
	outputParameters->connectedPlayers = connectedPlayers;
	pthread_create(&outputThread, NULL, &outputThreadHandler, outputParameters);
	slowPrint("\tOutput Thread is:\t\033[32;40mGREEN.\033[0m\n", delay);

	// Prepare the Scheme handler thread:
	SchemeThreadParameters * schemeParameters = malloc(sizeof(SchemeThreadParameters));
	schemeParameters->skillList = globalSkillList;
	schemeParameters->outputQueue = outputQueue;
	schemeParameters->areaList = areas;
	slowPrint("\tScheme Thread is:\t\033[32;40mGREEN.\033[0m\n", delay);
	slowPrint("=====\n", delay);
	pthread_create(&schemeThread, NULL, &schemeHandler, schemeParameters);
	
	while(true)
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
					//printf("Adding to list of sockets as %d.\n", index);
					gnutls_transport_set_int(tlssessions[index], clientSockets[index]);
					do 
					{
						returnVal = gnutls_handshake(tlssessions[index]);
					}
					while (returnVal < 0 && gnutls_error_is_fatal(returnVal) == 0);
					
					// Send a greeting message:
					memcpy(sendBuffer.senderName, "\0 Login > \0", 11);
					strcpy(sendBuffer.messageContent, "Welcome to the server!");
					messageSend(tlssessions[index], &sendBuffer);
					strcpy(receiveBuffer.messageContent, "/look");

					// Allocate the memory for a new input message:
					inputMessage * newMessage = malloc(sizeof(inputMessage));
					newMessage->content = malloc(sizeof(userMessage));

					// Copy in the correct data:
					memcpy(newMessage->content, &receiveBuffer, sizeof(userMessage));
					newMessage->sender = &connectedPlayers[index];

					// Push the new message onto the queue:
					pushQueue(inputQueue, newMessage, INPUT_MESSAGE);
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
						connectedPlayers[index].currentArea = getFromList(areas, 0)->area;

						// Prepare a fresh SSL session for the next new player:
						gnutls_init(&tlssessions[index], GNUTLS_SERVER);
						gnutls_priority_set_direct(tlssessions[index], "NORMAL:+ANON-ECDH:+ANON-DH", NULL);
						gnutls_credentials_set(tlssessions[index], GNUTLS_CRD_ANON, &serverkey);
						gnutls_handshake_set_timeout(tlssessions[index], GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
					}
					// Otherwise, they've sent a message:
					else 
					{
						// Allocate the memory for a new input message:
						inputMessage * newMessage = malloc(sizeof(inputMessage));
						newMessage->content = malloc(sizeof(userMessage));

						// Copy in the correct data:
						memcpy(newMessage->content, &receiveBuffer, sizeof(userMessage));
						newMessage->sender = &connectedPlayers[index];

						// Push the new message onto the queue:
						pushQueue(inputQueue, newMessage, INPUT_MESSAGE);
					}
				}			
			}
		}
	}
	pthread_cancel(gameLogicThread);
	pthread_cancel(outputThread);
	exit(EXIT_SUCCESS);
}
