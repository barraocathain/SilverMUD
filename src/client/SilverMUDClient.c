// Silverkin Industries Comm-Link Client, Public Demonstration Sample Alpha 0.5.
// PROJECT CODENAME: WHAT DO I PAY YOU FOR? | Level-3 Clearance.
// Barry Kane, 2021
#include <netdb.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <gnutls/gnutls.h>

#include "../queue.h"
#include "../constants.h"
#include "../playerdata.h"
#include "../texteffects.h"
#include "../inputoutput.h"

// A struct for bundling all needed parameters for a thread so we can pass them using a void pointer:
typedef struct threadparameters
{
	gnutls_session_t tlsSession;
	FILE * loggingStream;
	bool loggingFlag;
	WINDOW * window;
	int characterDelay;
	char * prompt; 
} threadparameters;

// Use sockaddr as a type:
typedef struct sockaddr sockaddr;

// A globally available exit boolean:
bool shouldExit = false;

// A function for managing the sending thread:
void * messageSender(void * parameters)	
{
	threadparameters * threadParameters = parameters;
	gnutls_session_t tlsSession = threadParameters->tlsSession;
	FILE * loggingStream = threadParameters->loggingStream;
	bool loggingFlag = threadParameters->loggingFlag;
	WINDOW * window = threadParameters->window;
	char * prompt = threadParameters->prompt;
	userMessage sendBuffer;

	// Repeatedly get input from the user, place it in a userMessage, and send it to the server:
	while (!shouldExit)
	{
		// Print the prompt:
		wprintw(window, "\n\n\n");
		wprintw(window, prompt);
		if (wgetnstr(window, sendBuffer.messageContent, MAX) == ERR)
		{
			// Quit if there's any funny business with getting input:
			pthread_exit(NULL);
		}
		
		// Ignore empty messages:
		if (sendBuffer.messageContent[0] == '\n')
		{
			continue;
		}
		
		// Send the message to the log if logging is enabled:
		if (loggingFlag == true)
		{
			fputs(sendBuffer.messageContent, loggingStream);
			fputs("\n", loggingStream);
			fflush(loggingStream);
		}

		// Send the message off to the server:
		messageSend(tlsSession, &sendBuffer);
		memset(&sendBuffer, 0, sizeof(char) * MAX);
	}

	// Rejoin the main thread:
	pthread_exit(NULL);
}

// A function for managing the receiving thread:
void * messageReceiver(void * parameters)
{
	threadparameters * threadParameters = parameters;
	gnutls_session_t tlsSession = threadParameters->tlsSession;
	FILE * loggingStream = threadParameters->loggingStream;
	int characterDelay = threadParameters->characterDelay;
	bool loggingFlag = threadParameters->loggingFlag;
	WINDOW * window = threadParameters->window;

	int returnValue = 0;
	userMessage receiveBuffer;
	bool serverMessage = false;
	int screenWidth = getmaxx(threadParameters->window);
	
	// Repeatedly take messages from the server and print them to the chat log window:
	while (!shouldExit) 
	{
		// Get the next message:
		returnValue = messageReceive(tlsSession, &receiveBuffer);
		
		// Check we haven't been disconnected:
		if (returnValue == -10 || returnValue == 0)
		{
			shouldExit = true;
		}

		// Check if it's a server message:
		else if (receiveBuffer.senderName[0] == '\0')
		{
			// Check if the server wants to change the prompt:
			if (receiveBuffer.senderName[1] != '\0')
			{
				strncpy(threadParameters->prompt, &receiveBuffer.senderName[1], 63);
				threadParameters->prompt[63] = '\0';
			}
			
			// Check if it's a command to disconnect:
			if (receiveBuffer.messageContent[0] == '\0' && receiveBuffer.senderName[1] != '\0') 
			{ 
				shouldExit = true; 
				pthread_exit(NULL); 
			}
			
			// Fit the string to the screen:
			wrapString(receiveBuffer.messageContent, strlen(receiveBuffer.messageContent) - 1, screenWidth);

			// If it's the first server message in a block, begin a block of server messages:
			if (serverMessage == false)
			{
				slowPrintNcurses("\n --====<>====--", characterDelay, window, true);
				serverMessage = true;
			}

			// Print the message:
			slowPrintNcurses("\n", characterDelay, window, true);
			slowPrintNcurses(receiveBuffer.messageContent, characterDelay,
							 window, false);
			slowPrintNcurses("\n", characterDelay, window, true);
		}
		// It's a user message:
		else
		{
			// Fit the string to the screen:
			wrapString(receiveBuffer.messageContent, strlen(receiveBuffer.messageContent) - 1,
					   screenWidth - strlen(receiveBuffer.senderName) - 2);

			// If the user has requested logging, insert the message into the file:
			if (loggingFlag == true)
			{
				fputs(receiveBuffer.senderName, loggingStream);
				fputs(": ", loggingStream);
				fputs(receiveBuffer.messageContent, loggingStream);
				fflush(loggingStream);
			}

			// If we're in a block of server messages, end it:
			if (serverMessage == true)
			{
				slowPrintNcurses("\n --====<>====-- \n", characterDelay, window, true);
				serverMessage = false;
			}
			
			// Print the message:
			slowPrintNcurses(receiveBuffer.senderName, characterDelay, window, true);
 			slowPrintNcurses(": ", characterDelay, window, true);
			slowPrintNcurses(receiveBuffer.messageContent, characterDelay, window, false);
		}
	}
	// Exit the thread if shouldExit is true:
	pthread_exit(NULL);
}
	
int main(int argc, char ** argv)
{
	int socketFileDesc;
	struct sockaddr_in serverAddress;
	pthread_t sendingThread;
	pthread_t receivingThread;
	int port = 5000;
	int currentopt = 0;
	int characterDelay = 4000;
	char chatLogPath[PATH_MAX + 1];
	char gameLogPath[PATH_MAX + 1];
	char ipAddress[32] = "127.0.0.1";
	FILE * chatLog = NULL, * gameLog = NULL;
	bool chatLogging = false, gameLogging = false;
	
	// Print welcome message:
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK CLIENT ====--\nVersion Alpha 0.5\n", 5000);

    // Parse command-line options:
	while ((currentopt = getopt(argc, argv, "i:c:g:p:d:")) != -1)
	{
		switch (currentopt)
		{
			case 'i':
			{
				memcpy(ipAddress, optarg, 32);
				break;
			}
			case 'c':
			{
				memcpy(chatLogPath, optarg, PATH_MAX + 1);
				chatLog = fopen(chatLogPath, "a+");
				if (chatLog == NULL)
				{
					chatLogging = false;
				}
				else
				{
					chatLogging = true;
				}
				break;
			}
			case 'g':
			{
				memcpy(gameLogPath, optarg, PATH_MAX + 1);
				gameLog = fopen(gameLogPath, "a+");
				if (gameLog == NULL)
				{
					gameLogging = false;
				}
				else
				{
					gameLogging = true;
				}
				break;
			}
			case 'p':
			{
				port = atoi(optarg);
				break;
			}
			case 'd':
			{
				characterDelay = atoi(optarg);
				break;
			}
			case '?':
			{
				return 1;
				break;
			}
		}
	}

	// Give me a socket, and make sure it's working:
	socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFileDesc == -1)
	{
		printf("Socket creation failed.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		slowPrint("Socket successfully created.\n", characterDelay);
	}
  
	// Set our IP address and port. Default to localhost for testing:
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ipAddress);
	serverAddress.sin_port = htons(port);
  
	// Connect the server and client sockets, Kronk:
	if (connect(socketFileDesc, (sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
	{
		slowPrint("Connection with the Silverkin Industries Comm-Link Server Failed:\nPlease contact your service representative.\n", characterDelay);
		exit(0);
	}

	// Setup a GnuTLS session and initialize it:
	gnutls_session_t tlsSession = NULL;
	if (gnutls_init(&tlsSession,  GNUTLS_CLIENT) < 0)
	{	
		exit(EXIT_FAILURE);
	}

	// Setup the private credentials for our GnuTLS session:
	gnutls_anon_client_credentials_t clientkey = NULL;
	gnutls_anon_allocate_client_credentials(&clientkey);
	gnutls_credentials_set(tlsSession, GNUTLS_CRD_ANON, &clientkey);

	// Bind the open socket to the TLS session:
	gnutls_transport_set_int(tlsSession, socketFileDesc);
	gnutls_priority_set_direct(tlsSession, "PERFORMANCE:+ANON-ECDH:+ANON-DH", NULL);

    // Use the default for the GnuTLS handshake timeout:
	gnutls_handshake_set_timeout(tlsSession, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

   	// Repeatedly attempt to handshake unless we encounter a fatal error:
	int returnValue = -1;
	do
	{
		returnValue = gnutls_handshake(tlsSession);
	}
	while (returnValue < 0 && gnutls_error_is_fatal(returnValue) == 0);

    // Setup Ncurses:
	initscr();
	
	// Create two pointers to structs to pass arguments to the threads:
	threadparameters * logArea;
	threadparameters * messageArea;
	
	logArea = malloc(sizeof(*logArea));
	messageArea = malloc(sizeof(*messageArea));
	
	// Make the windows for the structs, and pass the socket descriptor:
	logArea->window = newwin(LINES - 5, COLS - 2, 1, 1);
	logArea->tlsSession = tlsSession;
	logArea->loggingFlag = chatLogging;
	logArea->characterDelay = characterDelay;

	if (chatLog != NULL)
	{
		logArea->loggingStream = chatLog;
	}
	messageArea->window = newwin(3, COLS - 2, LINES - 4, 1);
	messageArea->tlsSession = tlsSession;
	messageArea->loggingFlag = gameLogging;
	
	// Set the appropriate log pointers:
	if (gameLog != NULL)
	{
		messageArea->loggingStream = gameLog;
	}

	// Set up the string to hold the current "prompt" that the server has sent:
	messageArea->prompt = calloc(64, sizeof(char));
	strcpy(messageArea->prompt, "> ");
	logArea->prompt = messageArea->prompt;

	// Set the two windows to scroll:
	scrollok(logArea->window, true);
	scrollok(messageArea->window, true);
	
	// Run a thread to send messages, and use another to recieve:
	pthread_create(&sendingThread, NULL, messageSender, messageArea);
	pthread_create(&receivingThread, NULL, messageReceiver, logArea);

	// Wait for /EXIT:
	pthread_join(receivingThread, NULL);

	// Close the threads:
	pthread_cancel(sendingThread);
	
	// Close the session and socket:
	gnutls_bye(tlsSession, GNUTLS_SHUT_WR);
	close(socketFileDesc);

	// Free the structs:
	free(logArea);
	free(messageArea);

	// Close the log files:
	if (gameLog != NULL)
	{
		fclose(gameLog);
	}
 	if (chatLog != NULL)
	{
		fclose(chatLog);
	}
	
	// Unsetup Ncurses:
	endwin();

	// Say goodbye:
	slowPrint("\nThank you for choosing Silverkin Industries, valued customer!\n", characterDelay);
}

