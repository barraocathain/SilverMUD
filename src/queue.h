// queue.h: Defines the queue data type and associated function prototypes for SilverMUD.
// Barry Kane, 2022
#ifndef QUEUE_H
#define QUEUE_H
#include "gamelogic.h"
#include "inputoutput.h"

// ========================
// -=[ Data Structures ]=-:
// ========================

// Let the compiler know there will be structs defined elsewhere:
typedef struct queue queue;

// An enum which is used to state what type of data is being stored in a queueNode:
typedef enum queueDataType
{
	EVENT,
	COMMAND,
    INPUT_MESSAGE,
	OUTPUT_MESSAGE
} queueDataType;

// A union for storing a pointer to all the types of data a queueNode may hold:
typedef union queueData
{
	outputMessage * outputMessage;
	inputMessage * inputMessage;
	commandEvent * command;
} queueData;

// A queue node, a singly-linked list node for our queue:
typedef struct queueNode queueNode;
typedef struct queueNode
{
	queueDataType type;
	queueData data;
	queueNode * next;
} queueNode;

// A queue, with pointers to the head and tail of the linked list, and data for multi-threading, locking, and an item count.
typedef struct queue
{
	volatile bool lock;
	size_t itemCount;
	queueNode * front;
	queueNode * back;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
} queue;

// ========================
// -=[    Functions    ]=-:
// ========================

// Allocates and instantiates a queue:
queue * createQueue(void);

// Destroys a queue and all of it's members:
void destroyQueue(queue ** queue);

// Returns the node at the front of the given queue:
queueNode * peekQueue(queue * queue);

// Removes the current node from the front of the queue:
void popQueue(queue * queue);

// Adds data to the back of a queue:
void pushQueue(queue * queue, void * data, queueDataType type);

#endif
