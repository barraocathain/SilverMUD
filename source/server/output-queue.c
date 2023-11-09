// =========================================
// | SilverMUD Server - output-queue.c     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "player-data.h"
#include "output-queue.h"

// A thread handler for constantly outputting messages from an output queue:
void * outputThreadHandler(void * outputQueue)
{
	struct OutputQueue * queue = (struct OutputQueue *)outputQueue;	
	struct OutputMessage * currentMessage = NULL;
	
	while (true)
	{
			currentMessage = popOutputMessage(queue);
			if (currentMessage != NULL)
			{
				struct PlayerListNode * currentPlayerNode = currentMessage->recepients->head;

				while (currentPlayerNode != NULL)
				{
					gnutls_record_send(*currentPlayerNode->player->connection->tlsSession,
									   currentMessage->message, sizeof(struct ServerToClientMessage));
					currentPlayerNode = currentPlayerNode->next;
				}

				if (currentMessage->deallocatePlayerList == true)
				{
					deallocatePlayerList(&currentMessage->recepients);
				}
			
				deallocateOutputMessage(&currentMessage);
			}
	}
}

struct OutputQueue * const createOutputQueue()
{
	// Allocate a new queue:
	struct OutputQueue * const newQueue = calloc(1, sizeof(struct OutputQueue));

	// Initialize it:
	pthread_mutex_init(&newQueue->mutex, NULL);
	newQueue->count = 0;
	newQueue->front = NULL;
	newQueue->back = NULL;

	// Return the new queue:
	return newQueue;
}

size_t pushOutputMessage(struct OutputQueue * const queue,
						 const bool deallocatePlayerList,
						 struct PlayerList * const recepients,
						 const enum MessageTypes type,
						 const char const * name, const char const * content,
						 const size_t nameLength, const size_t contentLength)
{
	// Allocate the appropriate memory for the queued message:
	struct OutputMessage * newMessage = calloc(1, sizeof(struct OutputMessage));
	newMessage->message = calloc(1, sizeof(struct ServerToClientMessage));

	// Copy in the appropriate values to the ServerToClientMessage:
	newMessage->message->type = type;
	
	strncpy(newMessage->message->name, name, (nameLength < MESSAGE_NAME_LENGTH) ?
			nameLength : MESSAGE_NAME_LENGTH);
	newMessage->message->name[MESSAGE_NAME_LENGTH - 1] = '\0';

	strncpy(newMessage->message->content, content, (contentLength < MESSAGE_CONTENT_LENGTH) ?
			contentLength : MESSAGE_CONTENT_LENGTH);
	newMessage->message->content[MESSAGE_CONTENT_LENGTH - 1] = '\0';

	// Copy in the appropriate values to the OutputMessage:
	newMessage->deallocatePlayerList = deallocatePlayerList;
	newMessage->recepients = recepients;

	// Entering critical section - Lock the queue:
	pthread_mutex_lock(&queue->mutex);

	// Add it to the queue:
	if (queue->back != NULL)
	{
		queue->back->next = newMessage;
		queue->back = newMessage;
	}
	
	if (queue->front == NULL)
	{
		queue->front = newMessage;
		queue->back = newMessage;
	}
	
	queue->count++;
	
	// Leaving critical section - Unlock the queue:
	pthread_mutex_unlock(&queue->mutex);

	return queue->count;
}

struct OutputMessage * popOutputMessage(struct OutputQueue * queue)
{
	if (queue->count == 0)
	{
		return NULL;
	}
	
	// Entering the critical section - Lock the queue:
	pthread_mutex_lock(&queue->mutex);

	struct OutputMessage * message = queue->front;
	queue->count--;

	if(queue->count == 0)
	{
		queue->front = NULL;
		queue->back = NULL;
	}

	else
	{
		queue->front = queue->front->next;
	}

	// Leaving the critical section - Unlock the queue:
	pthread_mutex_unlock(&queue->mutex);

	return message;
}

void deallocateOutputMessage(struct OutputMessage ** message)
{
	// Free and set the pointer to NULL:
	free(*message);
	message = NULL;
}

// ====================================================
// | End of output-queue.c, copyright notice follows. |
// ====================================================

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

