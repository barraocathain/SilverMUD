// Test for the queue type in SilverMUD:
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/queue.h"

#define formatBoolean(b) ((b) ? "true" : "false")
#define formatEquality(b) ((b) ? "equal" : "not equal")

int main(void)
{
	// Create a queue:
	printf("-=[ Creating queue ]=-:\n");
	queue * testQueue = createQueue();

	// Check that the queue has the correct values:
	printf("-=[ Checking initial values ]=-:\n");
	
	printf("- Item count should be 0:\n");
	assert(testQueue->itemCount == 0);
	printf("Item count is %d.\n\n", testQueue->itemCount);

	printf("- Lock should be false:\n");
	assert(testQueue->lock == false);
	printf("Lock is %s.\n\n", formatBoolean(testQueue->lock));

	printf("- Front should be (nil):\n");
	assert(testQueue->front == NULL);
	printf("Front is %p.\n\n", testQueue->front);
	
	printf("- Back should be (nil):\n");
	assert(testQueue->back == NULL);
	printf("Back is %p.\n\n", testQueue->front);

	// Create some items for the queue:
	inputMessage * testInputMessage = malloc(sizeof(inputMessage));
	testInputMessage->sender = NULL;
	testInputMessage->next = NULL;
	testInputMessage->content = malloc(sizeof(userMessage));
	strcpy(testInputMessage->content->senderName,"Queue Test Input Sender");
	strcpy(testInputMessage->content->messageContent, "Queue Test Input Content - Hello!");

	outputMessage * testOutputMessage = malloc(sizeof(outputMessage));
	for(int index = 0; index < PLAYERCOUNT; index++)
	{
		testOutputMessage->targets[index] = NULL;
	}
	testOutputMessage->next = NULL;
	testOutputMessage->content = malloc(sizeof(userMessage));
	strcpy(testOutputMessage->content->senderName, "Queue Test Output Sender");
	strcpy(testOutputMessage->content->messageContent, "Queue Test Output Content - World!");

 	commandEvent * testCommandEvent = malloc(sizeof(commandEvent));
	testCommandEvent->next = NULL;
	testCommandEvent->caller = NULL;
	testCommandEvent->command = malloc(5 * sizeof(char));
	testCommandEvent->arguments = malloc(15 * sizeof(char));
	strcpy(testCommandEvent->command, "Test");
	strcpy(testCommandEvent->arguments, "Test Arguments");

	// Add them to the queue:
	printf("-=[ Adding items to the queue ]=-:\n");
	printf("- First item, Item count should be 1. Front and Back should be equal.\n");
	pushQueue(testQueue, testInputMessage, INPUT_MESSAGE);
	assert(testQueue->itemCount == 1);
	assert(testQueue->front == testQueue->back);
	printf("Item count is: %d, Front and Back are %s.\n\n", testQueue->itemCount,
		   formatEquality(testQueue->front == testQueue->back));

	printf("- Second item, Item count should be 2. Front and Back should be not equal.\n");
	pushQueue(testQueue, testOutputMessage, OUTPUT_MESSAGE);
	assert(testQueue->itemCount == 2);
	assert(testQueue->front != testQueue->back);
	printf("Item count is: %d, Front and Back are %s.\n\n", testQueue->itemCount,
		   formatEquality(testQueue->front == testQueue->back));

	printf("- Third item, Item count should be 3. Front and Back should be not equal.\n");
	pushQueue(testQueue, testCommandEvent, COMMAND);
	assert(testQueue->itemCount == 3);
	assert(testQueue->front != testQueue->back);
	printf("Item count is: %d, Front and Back are %s.\n\n", testQueue->itemCount,
		   formatEquality(testQueue->front == testQueue->back));

	printf("-=[ Checking items and popping from queue ]=-:\n");
	printf("- First item peeked should point to testInputMessage.\n");
	assert(peekQueue(testQueue)->data.inputMessage == testInputMessage);        
	printf("Peeked data is located at: %p, testInputMessage is located at: %p.\n\n",
		   peekQueue(testQueue)->data.inputMessage, testInputMessage);

	printf("- Popping first item, Item count should be 2, Front and Back should not be equal.\n");
	popQueue(testQueue);
	assert(testQueue->itemCount == 2);
	assert(testQueue->front != testQueue->back);
	printf("Item count is: %d, Front and Back are %s.\n\n", testQueue->itemCount,
		   formatEquality(testQueue->front == testQueue->back));

	printf("- Second item peeked should point to testOutputMessage.\n");
	assert(peekQueue(testQueue)->data.outputMessage == testOutputMessage);
	printf("Peeked data is located at: %p, testOutputMessage is located at: %p.\n\n",
		   peekQueue(testQueue)->data.outputMessage, testOutputMessage);

	printf("- Popping second item, Item count should be 1, Front and Back should be equal.\n");
	popQueue(testQueue);
	assert(testQueue->itemCount == 1);
	assert(testQueue->front == testQueue->back);
	printf("Item count is: %d, Front and Back are %s.\n\n", testQueue->itemCount,
		   formatEquality(testQueue->front == testQueue->back));

	printf("- Third item peeked should point to testCommandEvent.\n");
	assert(peekQueue(testQueue)->data.command == testCommandEvent);
	printf("Peeked data is located at: %p, testCommandEvent is located at: %p.\n\n",
		   peekQueue(testQueue)->data.command, testCommandEvent);

	printf("- Popping third item:\n");
	popQueue(testQueue);
	
	printf("- Item count should be 0:\n");
	assert(testQueue->itemCount == 0);
	printf("Item count is %d.\n\n", testQueue->itemCount);

	printf("- Lock should be false:\n");
	assert(testQueue->lock == false);
	printf("Lock is %s.\n\n", formatBoolean(testQueue->lock));

	printf("- Front should be (nil):\n");
	assert(testQueue->front == NULL);
	printf("Front is %p.\n\n", testQueue->front);
	
	printf("- Back should be (nil):\n");
	assert(testQueue->back == NULL);
	printf("Back is %p.\n\n", testQueue->front);
}
