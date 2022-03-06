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
#include "misc/playerdata.h"
#include "misc/texteffects.h"
#include "misc/inputoutput.h"
#include "misc/inputhandling.h"
#define PORT 5000
#define SA struct sockaddr
static int MAX = 2048;

// A struct for passing arguments to our threads containing a file descriptor and a window pointer:
typedef struct threadparameters
{
	gnutls_session_t tlssession;
	FILE * loggingstream;
	bool loggingflag;
	WINDOW * window;
} threadparameters;

// A globally availible exit boolean.
bool shouldExit = false;

void * messageSender(void * parameters)	
{
	// Takes user input in a window, sanatizes it, and sends it to the server:
	struct threadparameters *threadParameters = parameters;
	userMessage sendBuffer;
  
	while (!shouldExit)
	{
		wprintw(threadParameters->window, "\n\n\nCOMM-LINK> ");
		if(wgetnstr(threadParameters->window, sendBuffer.messageContent, MAX) == ERR)
		{
			// Quit if there's any funny business with getting input:
			pthread_exit(NULL);
		}
		if(sendBuffer.messageContent[0] == '\n')
		{
			continue;
		}
		if(threadParameters->loggingflag == true)
		{
			fputs(sendBuffer.messageContent, threadParameters->loggingstream);
			fputs("\n", threadParameters->loggingstream);
			fflush(threadParameters->loggingstream);
		}
		wprintw(threadParameters->window, sendBuffer.messageContent);
		messageSend(threadParameters->tlssession, &sendBuffer);		
	}
	pthread_exit(NULL);
}


void * messageReceiver(void * parameters)
{
	// Takes messages from the server and prints them to the chat log window:
	struct threadparameters *threadParameters = parameters;
	userMessage receiveBuffer;
	while (!shouldExit) 
	{
		messageReceive(threadParameters->tlssession, &receiveBuffer);
		if(receiveBuffer.senderName[0] == '\0')
		{
			//if(receiveBuffer.messageContent[0] == '\0') 
			//{ 
			//	shouldExit = true; 
			//	pthread_exit(NULL); 
			//} 
			slowPrintNcurses("\n --====<>====-- \n", 8000, threadParameters->window);
			slowPrintNcurses(receiveBuffer.messageContent, 8000, threadParameters->window);
			slowPrintNcurses("\n --====<>====-- \n", 8000, threadParameters->window);
		}
		else
		{
			if(threadParameters->loggingflag == true)
			{
				fputs(receiveBuffer.senderName, threadParameters->loggingstream);
				fputs(": ", threadParameters->loggingstream);
				fputs(receiveBuffer.messageContent, threadParameters->loggingstream);
				fflush(threadParameters->loggingstream);
			}
			slowPrintNcurses(receiveBuffer.senderName, 8000, threadParameters->window);
			slowPrintNcurses(": ", 8000, threadParameters->window);
			slowPrintNcurses(receiveBuffer.messageContent, 8000, threadParameters->window);
		}
	}
	pthread_exit(NULL);
}
	
int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;
	pthread_t sendingThread;
	pthread_t receivingThread;
	int port = 5000;
	int currentopt = 0;
	char chatlogpath[PATH_MAX + 1];
	char gamelogpath[PATH_MAX + 1];
	char ipaddress[32] = "127.0.0.1";
	FILE * chatlog = NULL, * gamelog = NULL;
	bool chatlogging = false, gamelogging = false;

	// Print welcome message:
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK CLIENT ====--\nVersion Alpha 0.3\n", 5000);

    // Parse command-line options:
	while((currentopt = getopt(argc, argv, "i:c:g:p:")) != -1)
	{
		switch(currentopt)
		{
		case 'i':
		{
			strncpy(ipaddress, optarg, 32);
			break;
		}
		case 'c':
		{
			strncpy(chatlogpath, optarg, PATH_MAX + 1);
			chatlog = fopen(chatlogpath, "a+");
			if(chatlog == NULL)
			{
				chatlogging = false;
			}
			else
			{
				chatlogging = true;
			}
			break;
		}
		case 'g':
		{
			strncpy(gamelogpath, optarg, PATH_MAX + 1);
			gamelog = fopen(gamelogpath, "a+");
			if(gamelog == NULL)
			{
				gamelogging = false;
			}
			else
			{
				gamelogging = true;
			}
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
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed.\n");
		exit(0);
	}
	else
	{
		slowPrint("Socket successfully created.\n", 8000);
	}
	bzero(&servaddr, sizeof(servaddr));
  
	// Set our IP Address and port. Default to localhost for testing:
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ipaddress);
	servaddr.sin_port = htons(port);
  
	// Connect the server and client sockets, Kronk:
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0)
	{
		slowPrint("Connection with the Silverkin Industries Comm-Link Server Failed:\nPlease contact your service representative.\n", 8000);
		exit(0);
	}
	else
	{
		slowPrint("Connected to the Silverkin Industries Comm-Link Server:\nHave a pleasant day.\n", 8000);
	}
	usleep(100000);

	/* TODO: Negotiate TLS
	   Need to pull in GNU TLS, and do the same on the server-side. */
	// Setup a GnuTLS session and initialize it:
	gnutls_session_t tlssession = NULL;
	if(gnutls_init(&tlssession,  GNUTLS_CLIENT) < 0)
	{
		// Failure Case
		exit(EXIT_FAILURE);
	}
	gnutls_anon_client_credentials_t clientkey = NULL;
	gnutls_anon_allocate_client_credentials(&clientkey);
	gnutls_credentials_set(tlssession, GNUTLS_CRD_ANON, &clientkey);
	/* Bind the open socket to the TLS session. */
	gnutls_transport_set_int(tlssession, sockfd);
	gnutls_priority_set_direct(tlssession, "PERFORMANCE:+ANON-ECDH:+ANON-DH", NULL);

    /* Set default timeout for the handshake. */
	gnutls_handshake_set_timeout(tlssession, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
	int r = -1;
	do {
		r = gnutls_handshake(tlssession);
	} while (r < 0 && gnutls_error_is_fatal(r) == 0);

    // Setup Ncurses:
	initscr();

	// Create two pointers to structs to pass arguments to the threads:
	threadparameters * logArea;
	threadparameters * messageArea;
	
	logArea = malloc(sizeof(*logArea));
	messageArea = malloc(sizeof(*messageArea));

	// Make the windows for the structs, and pass the socket descriptor:
	logArea->window = newwin(LINES - 5, COLS - 2, 1, 1);
	logArea->tlssession = tlssession;
	logArea->loggingflag = chatlogging;
	if(chatlog != NULL)
	{
		logArea->loggingstream = chatlog;
	}
	messageArea->window = newwin(3, COLS, LINES - 3, 0);
	messageArea->tlssession = tlssession;
	messageArea->loggingflag = gamelogging;
	if(gamelog != NULL)
	{
		messageArea->loggingstream = gamelog;
	}
	
	// Set the two windows to scroll:
	scrollok(logArea->window, true);
	scrollok(messageArea->window, true);
	
	// Run a thread to send messages, and use main to recieve:
	pthread_create(&sendingThread, NULL, messageSender, messageArea);
	pthread_create(&receivingThread, NULL, messageReceiver, logArea);

	// Wait for /EXIT:
	pthread_join(receivingThread, NULL);

	// Close the threads:
	pthread_cancel(sendingThread);
	
	// Close the socket:
	close(sockfd);

	// Free the structs:
	free(logArea);
	free(messageArea);

	// Close the files:
	if(gamelog != NULL)
	{
		fclose(gamelog);
	}
 	if(chatlog != NULL)
	{
		fclose(chatlog);
	}
	
	// Unsetup Ncurses:
	endwin();

	// Say Goodbye:
	slowPrint("\nThank you for choosing Silverkin Industries, valued customer!\n", 8000);
}
