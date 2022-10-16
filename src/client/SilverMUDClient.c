// Silverkin Industries Comm-Link Client, Public Demonstration Sample Alpha 0.3.
// PROJECT CODENAME: WHAT DO I PAY YOU FOR? | Level-3 Clearance.
// Barry Kane, 2021
#include <netdb.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <gnutls/gnutls.h>
#include "../constants.h"
#include "../playerdata.h"
#include "../texteffects.h"
#include "../inputoutput.h"

// A struct for bundling all needed paramaters for a thread so we can pass them using a void pointer:
typedef struct threadparameters
{
	gnutls_session_t tlsSession;
	FILE * loggingStream;
	bool loggingFlag;
	WINDOW * window;
} threadparameters;

// Use sockaddr as a type:
typedef struct sockaddr sockaddr;

// A globally available exit boolean:
bool shouldExit = false;

void * messageSender(void * parameters)	
{
	struct threadparameters *threadParameters = parameters;
	userMessage sendBuffer;

	// Repeatedly get input from the user, place it in a userMessage, and send it to the server:
	while (!shouldExit)
	{
		// Print the prompt:
		wprintw(threadParameters->window, "\n\n\nCOMM-LINK> ");
		if (wgetnstr(threadParameters->window, sendBuffer.messageContent, MAX) == ERR)
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
		if (threadParameters->loggingFlag == true)
		{
			fputs(sendBuffer.messageContent, threadParameters->loggingStream);
			fputs("\n", threadParameters->loggingStream);
			fflush(threadParameters->loggingStream);
		}

		// Send the message off to the server:
		messageSend(threadParameters->tlsSession, &sendBuffer);		
	}
	pthread_exit(NULL);
}

void * messageReceiver(void * parameters)
{
	struct threadparameters *threadParameters = parameters;
	bool serverMessage = false;
	userMessage receiveBuffer;
	int screenWidth = getmaxx(threadParameters->window);
	
	// Repeatedly take messages from the server and print them to the chat log window:
	while (!shouldExit) 
	{
		messageReceive(threadParameters->tlsSession, &receiveBuffer);
		if (receiveBuffer.senderName[0] == '\0')
		{
			wrapString(receiveBuffer.messageContent,
					   strlen(receiveBuffer.messageContent) - 1, screenWidth);
			if (receiveBuffer.messageContent[0] == '\0') 
			{ 
				shouldExit = true; 
				pthread_exit(NULL); 
			}
			if(serverMessage == false)
			{
				slowPrintNcurses("\n --====<>====--", 4000, threadParameters->window, true);
				serverMessage = true;
			}
			slowPrintNcurses("\n", 4000, threadParameters->window, true);
			slowPrintNcurses(receiveBuffer.messageContent, 4000, threadParameters->window, false);
			slowPrintNcurses("\n", 4000, threadParameters->window, true);
		}
		else
		{
			wrapString(receiveBuffer.messageContent,
					   strlen(receiveBuffer.messageContent) - 1,
					   screenWidth - strlen(receiveBuffer.senderName) - 2);
			if (threadParameters->loggingFlag == true)
			{
				fputs(receiveBuffer.senderName, threadParameters->loggingStream);
				fputs(": ", threadParameters->loggingStream);
				fputs(receiveBuffer.messageContent, threadParameters->loggingStream);
				fflush(threadParameters->loggingStream);
			}
			if(serverMessage == true)
			{
				slowPrintNcurses("\n --====<>====-- \n", 4000, threadParameters->window, true);
				serverMessage = false;
			}
			slowPrintNcurses(receiveBuffer.senderName, 4000, threadParameters->window, true);
			slowPrintNcurses(": ", 4000, threadParameters->window, true);
			slowPrintNcurses(receiveBuffer.messageContent, 4000, threadParameters->window, false);
		}
	}
	pthread_exit(NULL);
}
	
int main(int argc, char **argv)
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
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK CLIENT ====--\nVersion Alpha 0.3\n", 5000);

    // Parse command-line options:
	while ((currentopt = getopt(argc, argv, "i:c:g:p:d:")) != -1)
	{
		switch (currentopt)
		{
		case 'i':
		{
			strncpy(ipAddress, optarg, 32);
			break;
		}
		case 'c':
		{
			strncpy(chatLogPath, optarg, PATH_MAX + 1);
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
			strncpy(gameLogPath, optarg, PATH_MAX + 1);
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
	bzero(&serverAddress, sizeof(serverAddress));
  
	// Set our IP Address and port. Default to localhost for testing:
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ipAddress);
	serverAddress.sin_port = htons(port);
  
	// Connect the server and client sockets, Kronk:
	if (connect(socketFileDesc, (sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
	{
		slowPrint("Connection with the Silverkin Industries Comm-Link Server Failed:\nPlease contact your service representative.\n", characterDelay);
		exit(0);
	}
	else
	{
		slowPrint("Connected to the Silverkin Industries Comm-Link Server:\nHave a pleasant day.\n", characterDelay);
	}
	usleep(100000);

	// Setup a GnuTLS session and initialize it:
	gnutls_session_t tlsSession = NULL;
	if (gnutls_init(&tlsSession,  GNUTLS_CLIENT) < 0)
	{
		// Failure Case
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
	if (chatLog != NULL)
	{
		logArea->loggingStream = chatLog;
	}
	messageArea->window = newwin(3, COLS - 2, LINES - 4, 1);
	messageArea->tlsSession = tlsSession;
	messageArea->loggingFlag = gameLogging;
	if (gameLog != NULL)
	{
		messageArea->loggingStream = gameLog;
	}
	
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

