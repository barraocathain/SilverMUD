// inputoutput.h: Header file contatning function prototypes and datastructures
// for dealing with input and output.
// Barry Kane, 2022.
#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include "constants.h"
#include "playerdata.h"
#include <gnutls/gnutls.h>

// A message datastructure containing a user/character name and the content:
typedef struct userMessage
{	
	char senderName[32];
	char messageContent[MAX];
} userMessage;

// ==================
// -=[Message I/O]=-:
// ==================

// Sends a message to a given TLS session, wraps the calls to gnutls_write:
int messageSend(gnutls_session_t receivingSession, userMessage * messageToSend);

// Receives a message from a given TLS session, wraps the calls to gnutls_read:
int messageReceive(gnutls_session_t receiveFromSession, userMessage * receiveToMessage);

// ===================
// -=[Output Queue]=-:
// ===================
typedef struct outputMessage outputMessage;
typedef struct outputMessage
{
	outputMessage * next;
	playerInfo * targets[PLAYERCOUNT];
	userMessage * content;
} outputMessage;

// A first-in first-out queue for message output to players:
typedef struct outputMessageQueue
{
	bool lock;
	int currentLength;
	outputMessage * back;
	outputMessage * front;
} outputMessageQueue;

// Creates and initializes a outputMessageQueue:
outputMessageQueue * createOutputMessageQueue(void);

// Enqueue a userMessage to an outputMessageQueue:
int queueOutputMessage(outputMessageQueue * queue, userMessage messageToQueue);
int queueTargetedOutputMessage(outputMessageQueue * queue, userMessage *  messageToQueue,
							   playerInfo ** targets, int numberOfTargets);

// Dequeue the front outputMessage from an outputMessageQueue:
int dequeueOutputMessage(outputMessageQueue * queue);

// Return the front outputMessage from an outputMessageQueue:
outputMessage * peekOutputMessage(outputMessageQueue * queue);

// ==================
// -=[Input Queue]=-:
// ==================
typedef struct inputMessage inputMessage;
typedef struct inputMessage
{
	playerInfo * sender;
	userMessage * content;
} inputMessage;

// =======================
// -=[Input Sanitation]=-:
// =======================

// Sanatize user input to ensure it's okay to send to the server:
void userInputSanatize(char * inputString, int length);

// Sanatize user names so they display correctly;
void userNameSanatize(char * inputString, int length);

#endif

