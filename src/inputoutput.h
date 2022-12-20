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
	int recipientsCount;
	userMessage * content;
	playerInfo ** recipients;
} outputMessage;

// Create a targetedOutput message to be delivered to the players pointed to in recipients:
outputMessage * createTargetedOutputMessage(userMessage * messageToQueue, playerInfo ** recipients, int recipientCount);

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

