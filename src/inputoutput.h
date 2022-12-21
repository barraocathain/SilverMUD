// inputoutput.h: Header file contatning function prototypes and datastructures
// for dealing with input and output.
// Barry Kane, 2022.
#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gnutls/gnutls.h>

#include "queue.h"
#include "constants.h"
#include "playerdata.h"

// Let the compiler know there will be structs defined elsewhere:
typedef struct queue queue;

// ========================
// -=[ Data Structures ]=-:
// ========================

// Contains a character/player name and the content of a message:
typedef struct userMessage
{	
	char senderName[32];
	char messageContent[MAX];
} userMessage;

// Contains a userMessage and a pointer to the playerInfo of the connection which sent it:
typedef struct inputMessage
{
	playerInfo * sender;
	userMessage * content;
} inputMessage;

// Contains a userMessage and pointers to the playerInfo of the recipients and the number of them:
typedef struct outputMessage
{
	int recipientsCount;
	userMessage * content;
	playerInfo ** recipients;
} outputMessage;

// Contains the relevant parameters for the outputThreadLoop function:
typedef struct outputThreadParameters
{
	queue * outputQueue;
	gnutls_session_t * tlssessions;
	playerInfo * connectedPlayers;
} outputThreadParameters;

// ========================
// -=[    Functions    ]=-:
// ========================

// Sends a message to a given TLS session, wraps the calls to gnutls_write:
int messageSend(gnutls_session_t receivingSession, userMessage * messageToSend);

// Receives a message from a given TLS session, wraps the calls to gnutls_read:
int messageReceive(gnutls_session_t receiveFromSession, userMessage * receiveToMessage);

// Create a targetedOutput message to be delivered to the players pointed to in recipients:
outputMessage * createTargetedOutputMessage(userMessage * messageToQueue, playerInfo ** recipients, int recipientCount);

// A function for the output thread, which sends queued messages:
void * outputThreadHandler(void * parameters);

// Sanatize user input to ensure it's okay to process:
void userInputSanatize(char * inputString, int length);

// Sanatize user names so they display correctly:
void userNameSanatize(char * inputString, int length);

#endif

