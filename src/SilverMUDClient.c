#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "misc/texteffects.h"
#define MAX 1024
#define PORT 5000
#define SA struct sockaddr

void * messageSender(void * sockfd)	
{
	char sendBuffer[MAX];
	int characterindex;

	while (1)
	{
		bzero(sendBuffer, MAX);
		printf("COMM-LINK> ");
		fgets(sendBuffer, MAX, stdin);
		if(sendBuffer[0] != '\n');
		{
			write((long)sockfd, sendBuffer, MAX);
		}
        }
}


void * messageReceiver(void * sockfd)
{
	char receiveBuffer[MAX];
	while (1)
	{
		read((long)sockfd, receiveBuffer, MAX);
		slowprint("\nUSER-MESSAGE: ", 8000);
		slowprint(receiveBuffer, 8000);
		slowprint("\nCOMM-LINK (CONT.)> ", 8000);
		bzero(receiveBuffer, MAX);
	}
}
	
int main(int argc, char **argv)
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
	pthread_t messagingThread;
	
	// Give me a socket, and make sure it's working:
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed.\n");
		exit(0);
	}
	else
	{
		slowprint("Socket successfully created.\n", 8000);
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
		slowprint("Connection with the Silverkin Industries Comm-Link Server Failed:\nPlease contact your service representative.\n", 8000);
		exit(0);
	}
	else
	{
		slowprint("Connected to the Silverkin Industries Comm-Link Server:\nHave a pleasant day.\n", 8000);
	}
	
	// Run a thread to send messages, and use main to recieve.
	pthread_create(&messagingThread, NULL, messageSender, (void *)(long)sockfd);
	messageReceiver((void *)(long)sockfd);
	
	// Close the socket.
	close(sockfd);
}
