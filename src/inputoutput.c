// inputoutput.c: Implementation of input/output library for SilverMUD.
// Barry Kane, 2022.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <gnutls/gnutls.h>

#include "queue.h"
#include "constants.h"
#include "playerdata.h"
#include "inputoutput.h"

// Sends a message to a given TLS session, wraps the calls to gnutls_write:
int messageSend(gnutls_session_t receivingSession, userMessage * messageToSend)
{
	int returnValue = 0;
	// Continuously attempt to send the name field until it succeeds or fatally errors:
	do
	{
		returnValue = gnutls_record_send(receivingSession, messageToSend->senderName,
										 sizeof(((userMessage*)0)->senderName));
	}  while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);

    // Continuously attempt to send the message field until it succeeds or fatally errors:
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
	// Continuously attempt to receive the name field until it succeeds or fatally errors:
	do
	{
		returnValue = gnutls_record_recv(receiveFromSession, receiveToMessage->senderName,
										sizeof(((userMessage*)0)->senderName));
	}  while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);

	// Continuously attempt to receive the message field until it succeeds or fatally errors:
	do
	{
		returnValue = gnutls_record_recv(receiveFromSession, receiveToMessage->messageContent,
										 sizeof(((userMessage*)0)->messageContent));
	} while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
	
	return returnValue;
}

// Allocate and initialize an outputMessage targeted to a variable amount of players:
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

// A function for the output thread, which sends queued messages:
void * outputThreadHandler(void * parameters)
{
	outputThreadParameters * variables = parameters;
   	queue * outputQueue = variables->outputQueue;
	gnutls_session_t * tlssessions = variables->tlssessions;
	playerInfo * connectedPlayers = variables->connectedPlayers;
	
	while (true)
	{
		// If there's nothing to do, put the thread to sleep:
		if (outputQueue->itemCount == 0)
		{
			pthread_cond_wait(&outputQueue->condition, &outputQueue->mutex);
		}
		
		// Run through the output queue and send all unsent messages:
		while (outputQueue->itemCount != 0)
		{
			// Wait until the queue unlocks:
			while (outputQueue->lock);
			
			// Lock the queue:
			outputQueue->lock = true;

			// Get a message off the queue:
			outputMessage * message = peekQueue(outputQueue)->data.outputMessage;

			// Unlock the queue:
			outputQueue->lock = false;
			
			// If the first target is set to NULL, it's intended for all connected:
			if (message->recipientsCount == 0)
			{
				for (int index = 0; index < PLAYERCOUNT; index++)  
				{
					messageSend(tlssessions[index], message->content);
				}
			}

			// Otherwise, send it only to the targeted players:
			else
			{
				int sentCount = 0;
				for (int index = 0; index < PLAYERCOUNT; index++)
				{
					if (sentCount == message->recipientsCount)
					{
						break;
					}
					if (&connectedPlayers[index] == message->recipients[targetIndex])
					{
						sentCount++;
						messageSend(tlssessions[index], message->content);
					}
				}
			}

			// Remove the output message from the queue:
			popQueue(outputQueue);
		}
	}		
}

// Sanatize user input to ensure it's okay to process:
void userInputSanatize(char * inputString, int length)
{
	for (int index = 0; index <= length; index++)
	{
		// If it's not a printable character, it has no business being here:
		if(!isprint(inputString[index]))
		{
			inputString[index] = '\n';
			inputString[index + 1] = '\0';
			break;
		}
	}
	
	// Make sure it's null-terminated:
	inputString[length - 1] = '\0';
}

// Sanatize user names so they display correctly;
void userNameSanatize(char * inputString, int length)
{
	for(int index = 0; index <= length; index++)
	{
		// If it's not a printable character, it has no business being here:
		if(!isprint(inputString[index]))
		{
			inputString[index] = '\0';
			break;
		}
	}
	// Make sure it's null-terminated:
	inputString[length - 1] = '\0';
}
