#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "misc/playerdata.h"
#include "misc/texteffects.h"
#include "misc/inputhandling.h"
#define MAX 1024
#define PORT 5000
#define SA struct sockaddr

// A struct for passing arguments to our threads containing a file descriptor and a window pointer:
typedef struct threadparameters
{
	int socketDescriptor;
	WINDOW * window;
} threadparameters;

// A globally availible exit boolean.
bool shouldExit = false;

void sigintHandler(int signal)
{
	shouldExit = true;
}

void * messageSender(void * parameters)	
{
	// Takes user input in a window, sanatizes it, and sends it to the server:
	struct threadparameters *threadParameters = parameters;
	char sendBuffer[MAX];
	int characterindex;
  
	while (!shouldExit)
	{
		bzero(sendBuffer, MAX);
		wprintw(threadParameters->window, "\n\n\nCOMM-LINK> ");
		if(wgetnstr(threadParameters->window, sendBuffer, MAX) == ERR)
		{
			// Quit if there's any funny business with getting input:
			pthread_exit(NULL);
		}
		userInputSanatize(sendBuffer, MAX);
		if(sendBuffer[0] == '\n')
		{
			continue;
		}
		write(threadParameters->socketDescriptor, sendBuffer, MAX);		
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
		read(threadParameters->socketDescriptor, &receiveBuffer.senderName, sizeof(receiveBuffer.senderName));
		read(threadParameters->socketDescriptor, &receiveBuffer.messageContent, sizeof(receiveBuffer.messageContent));
		slowPrintNcurses(receiveBuffer.senderName, 8000, threadParameters->window);
		slowPrintNcurses(": ", 8000, threadParameters->window);
		slowPrintNcurses(receiveBuffer.messageContent, 8000, threadParameters->window);
		bzero(receiveBuffer.senderName, sizeof(receiveBuffer.senderName));
		bzero(receiveBuffer.messageContent, sizeof(receiveBuffer.messageContent));
	}
	pthread_exit(NULL);
}
	
int main(int argc, char **argv)
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
	pthread_t sendingThread;
	pthread_t receivingThread;
	
	// Set the SIGINT handler.
	signal(SIGINT, sigintHandler);

	// Print welcome message:
	slowPrint("\n--==== \033[33;40mSILVERKIN INDUSTRIES\033[0m COMM-LINK CLIENT ====--\nVersion Alpha 0.3\n", 5000);
	
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
	if (argc == 1)
	{
		servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}
	else
	{
		servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	}
	servaddr.sin_port = htons(PORT);
  
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
	
	// Setup Ncurses:
	initscr();

	// Create two pointers to structs to pass arguments to the threads:
	threadparameters * logArea;
	threadparameters * messageArea;
	
	logArea = malloc(sizeof(*logArea));
	messageArea = malloc(sizeof(*messageArea));

	// Make the windows for the structs, and pass the socket descriptor:
	logArea->window = newwin(LINES - 5, COLS - 2, 1, 1);
	logArea->socketDescriptor = sockfd;
	messageArea->window = newwin(3, COLS, LINES - 3, 0);
	messageArea->socketDescriptor = sockfd;

	// Set the two windows to scroll:
	scrollok(logArea->window, true);
	scrollok(messageArea->window, true);
	
	// Run a thread to send messages, and use main to recieve:
	pthread_create(&sendingThread, NULL, messageSender, messageArea);
	pthread_create(&receivingThread, NULL, messageReceiver, logArea);

	// Wait for SIGINT to change
	while(!shouldExit)
	{
		sleep(250);
	}

	// Close the threads:
	pthread_cancel(sendingThread);
	pthread_cancel(receivingThread);
	
	// Close the socket:
	close(sockfd);

	// Free the structs:
	free(logArea);
	free(messageArea);
	
	// Unsetup Ncurses:
	endwin();

	// Say Goodbye:
	slowPrint("\nThank you for choosing Silverkin Industries, valued customer!\n", 8000);
}
