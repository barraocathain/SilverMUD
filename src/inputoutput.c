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

outputMessageQueue * createOutputMessageQueue(void)
{
	outputMessageQueue * newQueue = malloc(sizeof(outputMessageQueue));
	newQueue->front = NULL;
	newQueue->back = NULL;
	newQueue->currentLength = 0;
	newQueue->lock = false;
	return newQueue;
}

int queueOutputMessage(outputMessageQueue * queue, userMessage messageToQueue)
{
	// Copy the message into a new output message:
	outputMessage * newOutputMessage = malloc(sizeof(outputMessage));

	// Allocate the internal userMessage to store the message:
	newOutputMessage->content = malloc(sizeof(userMessage));
	
	// Copy the userMessage to the internal userMessage:
	strncpy(newOutputMessage->content->senderName, messageToQueue.senderName, 32);
	strncpy(newOutputMessage->content->messageContent, messageToQueue.messageContent, MAX);

	// We have no targets, NULL sends to all players in an area:
	newOutputMessage->targets[0] = NULL;
	
	// Wait for the queue to unlock:
	while (queue->lock);

	// Lock the queue:
	queue->lock = true;

	// Check that we're not overflowing the queue:
	if ((queue->currentLength + 1) > MAXQUEUELENGTH)
	{
		// Unlock the queue:
		queue->lock = false;
		return -1;
	}
	else
	{
		// If the queue is empty, set the first message as both the front and back of the queue:
		if(queue->front == NULL)
		{
			queue->front = newOutputMessage;
			queue->back = newOutputMessage;
			queue->currentLength++;
			
			// Unlock the queue:
			queue->lock = false;
			return 0;
		}
		else
		{
			queue->back->next = newOutputMessage;
			queue->back = newOutputMessage;
			queue->currentLength++;

            // Unlock the queue:
			queue->lock = false;
			return 0;
		}
	}
}

int queueTargetedOutputMessage(outputMessageQueue * queue,
							   userMessage *  messageToQueue, playerInfo ** targets, int numberOfTargets)
{
	// Copy the message into a new output message:
	outputMessage * newOutputMessage = malloc(sizeof(outputMessage));

	// Allocate the internal userMessage to store the message:
	newOutputMessage->content = malloc(sizeof(userMessage));

	// Set the appropriate recipients:
	for(int index = 0; index < numberOfTargets && index < PLAYERCOUNT; index++)
	{
		newOutputMessage->targets[index] = targets[index];
	}
	for(int index = numberOfTargets; index < PLAYERCOUNT; index++)
	{
		newOutputMessage->targets[index] = NULL;
	}
	
	// Copy the userMessage to the internal userMessage:
	strncpy(newOutputMessage->content->senderName, messageToQueue->senderName, 32);
	strncpy(newOutputMessage->content->messageContent, messageToQueue->messageContent, MAX);
		
	// Wait for the queue to unlock:
	while (queue->lock);

	// Lock the queue:
	queue->lock = true;

	// Check that we're not overflowing the queue:
	if ((queue->currentLength + 1) > MAXQUEUELENGTH)
	{
		// Unlock the queue:
		queue->lock = false;
		return -1;
	}
	else
	{
		// If the queue is empty, set the first message as both the front and back of the queue:
		if(queue->front == NULL)
		{
			queue->front = newOutputMessage;
			queue->back = newOutputMessage;
			queue->currentLength++;
			
			// Unlock the queue:
			queue->lock = false;
			return 0;
		}
		else
		{
			queue->back->next = newOutputMessage;
			queue->back = newOutputMessage;
			queue->currentLength++;

            // Unlock the queue:
			queue->lock = false;
			return 0;
		}
	}
}

int dequeueOutputMessage(outputMessageQueue * queue)
{
	// Wait for the queue to unlock:
	while (queue->lock);

	// Lock the queue:
	queue->lock = true;

	// Check the list isn't empty:
	if(queue->front == NULL)
	{
		queue->lock = false;
		return -1;
	}
	
	// If there is only one item in the queue:
	else if(queue->front == queue->back)
	{
		free(queue->front->content);
		free(queue->front);
		queue->front = NULL;
		queue->back = NULL;
		queue->currentLength--;
		queue->lock = false;
		return 0;
	}

	// Remove the front item:
	else
	{
		outputMessage * messageToDelete = queue->front;
		queue->front = queue->front->next;
		free(messageToDelete->content);
		free(messageToDelete);
		queue->currentLength--;
		queue->lock = false;
		return 0;
	}
}

inputMessageQueue * createInputMessageQueue(void)
{
	inputMessageQueue * newQueue = malloc(sizeof(inputMessageQueue));
	newQueue->front = NULL;
	newQueue->back = NULL;
	newQueue->currentLength = 0;
	newQueue->lock = false;
	return newQueue;
}

int dequeueInputMessage(inputMessageQueue * queue)
{
	// Wait for the queue to unlock:
	while (queue->lock);

	// Lock the queue:
	queue->lock = true;

	// Check the list isn't empty:
	if(queue->front == NULL)
	{
		queue->lock = false;
		return -1;
	}
	
	// If there is only one item in the queue:
	else if(queue->front == queue->back)
	{
		free(queue->front->content);
		free(queue->front);
		queue->front = NULL;
		queue->back = NULL;
		queue->currentLength--;
		queue->lock = false;
		return 0;
	}

	// Remove the front item:
	else
	{
		inputMessage * messageToDelete = queue->front;
		queue->front = queue->front->next;
		free(messageToDelete->content);
		free(messageToDelete);
		queue->currentLength--;
		queue->lock = false;
		return 0;
	}
}

int queueInputMessage(inputMessageQueue * queue, userMessage messageToQueue, playerInfo * sendingPlayer)
{
	// Copy the message into a new input message:
	inputMessage * inputMessage = malloc(sizeof(inputMessage));

	// Allocate the internal userMessage to store the message:
	inputMessage->content = malloc(sizeof(userMessage));

	// Copy the userMessage to the internal userMessage:
	strncpy(inputMessage->content->senderName, messageToQueue.senderName, 32);
	strncpy(inputMessage->content->messageContent, messageToQueue.messageContent, MAX);

	// We have no targets, NULL sends to all players in an area:
	inputMessage->sender = sendingPlayer;	

	// Wait for the queue to unlock:
	while (queue->lock);

	// Lock the queue:
	queue->lock = true;

	// Check that we're not overflowing the queue:
	if ((queue->currentLength + 1) > MAXQUEUELENGTH)
	{
		// Unlock the queue:
		queue->lock = false;
		return -1;
	}
	else
	{
		// If the queue is empty, set the first message as both the front and back of the queue:
		if(queue->front == NULL)
		{
			queue->front = inputMessage;
			queue->back = inputMessage;
			queue->currentLength++;
			
			// Unlock the queue:
			queue->lock = false;

			return 0;
		}
		else
		{
			queue->back->next = inputMessage;
			queue->back = inputMessage;
			queue->currentLength++;

			// Unlock the queue:
			queue->lock = false;

			return 0;
		}
	}
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

// Return the front inputMessage from an inputMessageQueue:
inputMessage * peekInputMessage(inputMessageQueue * queue)
{
	return queue->front;
}

outputMessage * peekOutputMessage(outputMessageQueue * queue)
{
	return queue->front;
}
