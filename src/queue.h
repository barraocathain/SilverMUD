// queue.h: Defines the queue data type and associated function prototypes for SilverMUD.
// Barry Kane, 2022
#ifndef QUEUE_H
#define QUEUE_H
#include "gamelogic.h"
#include "inputoutput.h"

// ========================
// -=[ Data Structures ]=-:
// ========================

typedef enum queueDataType
{
	EVENT,
	COMMAND,
    INPUT_MESSAGE,
	OUTPUT_MESSAGE
} queueDataType;

typedef union queueData
{
	outputMessage * outputMessage;
	inputMessage * inputMessage;
	commandEvent * command;
} queueData;

typedef struct queueNode queueNode;
typedef struct queueNode
{
	queueDataType type;
	queueData data;
	queueNode * next;
} queueNode;

typedef struct queue
{
	bool lock;
	size_t itemCount;
	queueNode * front;
	queueNode * back;
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
