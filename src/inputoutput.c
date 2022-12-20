// inputoutput.c: Implementation of input/output library for SilverMUD.
// Barry Kane, 2022.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gnutls/gnutls.h>
#include "constants.h"
#include "playerdata.h"
#include "inputoutput.h"

// Sends a message to a given TLS session, wraps the calls to gnutls_write:
int messageSend(gnutls_session_t receivingSession, userMessage * messageToSend)
{
	int returnValue = 0;
	do
	{
		returnValue = gnutls_record_send(receivingSession, messageToSend->senderName,
										 sizeof(((userMessage*)0)->senderName));
	}  while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
	do
	{
		returnValue = gnutls_record_send(receivingSession, messageToSend->messageContent,
										 sizeof(((userMessage*)0)->messageContent));
	} while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);

	return returnValue;
}

// Recieves a message from a given TLS session, wraps the calls to gnutls_read:
int messageReceive(gnutls_session_t receiveFromSession, userMessage * receiveToMessage)
{
	int returnValue = 0;
	do
	{
		returnValue = gnutls_record_recv(receiveFromSession, receiveToMessage->senderName,
										sizeof(((userMessage*)0)->senderName));
	}  while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
	do
	{
		returnValue = gnutls_record_recv(receiveFromSession, receiveToMessage->messageContent,
										 sizeof(((userMessage*)0)->messageContent));
	} while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
	
	return returnValue;
}

outputMessage * createTargetedOutputMessage(userMessage * messageToQueue, playerInfo ** recipients, int recipientsCount)
{
	// Allocate a new output message:
	outputMessage * newOutputMessage = malloc(sizeof(outputMessage));
	newOutputMessage->content = malloc(sizeof(userMessage));

	// Allocate an array of playerInfo for the output message recepients:
	newOutputMessage->recipients = malloc(sizeof(playerInfo*) * recipientsCount);

	// Copy in the appropriate data:
	memcpy(newOutputMessage->recipients, recipients, sizeof(playerInfo *) * recipientsCount);
	memcpy(newOutputMessage->content, messageToQueue, sizeof(userMessage));
	newOutputMessage->recipientsCount = recipientsCount;

	// Return a pointer to the new outputMessage:
	return newOutputMessage;
}

void userInputSanatize(char * inputString, int length)
{
	for(int index = 0; index <= length; index++)
	{
		if(!isprint(inputString[index]))
		{
			inputString[index] = '\n';
			inputString[index + 1] = '\0';
			break;
		}
	}
	inputString[length - 1] = '\0';
}

void userNameSanatize(char * inputString, int length)
{
	for(int index = 0; index <= length; index++)
	{
		if(!isprint(inputString[index]))
		{
			inputString[index] = '\0';
			break;
		}
	}
	inputString[length - 1] = '\0';
}
