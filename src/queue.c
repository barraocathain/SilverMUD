// queue.c: Implements the queue data type and associated functions for SilverMUD.
// Barry Kane, 2022
#include <pthread.h>
#include "queue.h"

// Allocates and instantiates a queue:
queue * createQueue(void)
{
	// Allocate the memory for the queue:
	queue * newQueue = malloc(sizeof(queue));

	// Instantiate the variables in the data-structure:
	newQueue->itemCount = 0;
	newQueue->front = NULL;
	newQueue->back = NULL;

	// Create the threading constructs:
	newQueue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;   
	newQueue->condition = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
	
	// Return the pointer to the new queue:
	return newQueue;
}

// Destroys a queue and all of it's members:
void destroyQueue(queue ** queue)
{
	// Pop everything off of the queue:
	while ((*queue)->itemCount > 0)
	{
		popQueue(*queue);
	}

	// Deallocate the queue:
	free(*queue);

	// Point the queue pointer to NULL;
	*queue = NULL;
}

// Returns the data at the front of the given queue:
queueNode * peekQueue(queue * queue)
{
	return queue->front;
}

// Removes the current data from the front of the queue:
void popQueue(queue * queue)
{
	// Check if the queue is locked, and wait:
	while (queue->lock);
	
	// Lock the queue:
	queue->lock = true;

	// Check there is actually anything to remove:
	if (queue->itemCount == 0)
	{
		queue->lock = false;
		return;
	}
	
	// Handle the special case of being the last item in the queue:
	else if (queue->itemCount == 1)
	{
		// Deallocate the correct data:
		switch (queue->front->type)
		{
			case EVENT:
			{
				// TODO: Implement events.
			}
			case COMMAND:
			{
				free(queue->front->data.command->command);
				free(queue->front->data.command->arguments);
				free(queue->front->data.command);
				break;
			}
			case INPUT_MESSAGE:
			{
				free(queue->front->data.inputMessage->content);
				free(queue->front->data.inputMessage);
				break;
			}
			case OUTPUT_MESSAGE:
			{
				free(queue->front->data.outputMessage->content);
				free(queue->front->data.outputMessage);
				break;
			}
		}

		// Deallocate the node:
		free(queue->front);

		// Set the correct variables for the queue:
		queue->front = NULL;
		queue->back = NULL;
		queue->itemCount = 0;

		// Unlock the queue:
		queue->lock = false;

		return;
	}

	// Remove the current front of the queue:
	else
	{
		// Deallocate the correct data:
		switch (queue->front->type)
		{
			case EVENT:
			{
				// TODO: Implement events.
				break;
			}
			case COMMAND:
			{
				free(queue->front->data.command->command);
				free(queue->front->data.command->arguments);
				free(queue->front->data.command);
				break;
			}
			case INPUT_MESSAGE:
			{
				free(queue->front->data.inputMessage->content);
				free(queue->front->data.inputMessage);
				break;
			}
			case OUTPUT_MESSAGE:
			{
				free(queue->front->data.outputMessage->content);
				free(queue->front->data.outputMessage);
				break;
			}
		}

		// Save a pointer to the current node so we don't leak it:
		queueNode * nodeToDelete = queue->front;

		// Advance the queue:
		queue->front = queue->front->next;
		queue->itemCount--;
		
		// Deallocate the old node:
		free(nodeToDelete);

		// Unlock the queue:
		queue->lock = false;
		
		return;
	}
}

// Adds data to the back of a queue:
void pushQueue(queue * queue, void * data, queueDataType type)
{
	// Check if the queue is locked:
	while (queue->lock);

	// Create a new node:
	queueNode * newNode = malloc(sizeof(queueNode));
	newNode->type = type;
	// Copy the data into the correct slot for the type:
	switch (type)
	{
		case EVENT:
		{
			// TODO: Implement events.
			break;
		}		
		case COMMAND:
		{
			newNode->data.command = data;
			break;
		}
		case INPUT_MESSAGE:
		{
			newNode->data.inputMessage = data;
			break;
		}
		case OUTPUT_MESSAGE:
		{
			newNode->data.outputMessage = data;
			break;
		}

	}

	// Check if the queue is locked:
	while (queue->lock);

	// Lock the queue:
	queue->lock = true;

	// Set the correct pointers:
	newNode->next = NULL;
	
	if (queue->itemCount == 0)
	{
		queue->front = newNode;
		queue->back = newNode;
	}
	else
	{
		queue->back->next = newNode;
		queue->back = newNode;
	}
	
	// Increase the queue item count:
	queue->itemCount++;

	// Unlock the queue:
	queue->lock = false;

	// Flag that the queue was modified:
	pthread_cond_broadcast(&queue->condition);
}
