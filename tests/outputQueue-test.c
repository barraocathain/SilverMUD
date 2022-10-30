#include "../src/inputoutput.h"
#include <stdio.h>
#include <string.h>
int main()
{
	userMessage A, B, C;
	strncpy(A.senderName, "Bob\0", 32 -1);
	strncpy(A.messageContent, "joins the fray!\0", MAX-1);
	strncpy(B.senderName, "Alice\0", 32 -1);
	strncpy(B.messageContent, "challenges the unknown!\0", MAX -1);
	strncpy(C.senderName, "Tom\0", 32 -1);
	strncpy(C.messageContent, "Attacks them all!\0", MAX -1);
	outputMessageQueue * testQueue = createOutputMessageQueue();
	printf("Queue Created.\n");
	printf("%d", queueOutputMessage(testQueue, A));
	printf("Queued A.\n");
	printf("%d", queueOutputMessage(testQueue, B));
	printf("Queued B.\n");
	printf("%d", queueOutputMessage(testQueue, C));
	printf("Queued C.\n");
	printf("%s\n", testQueue->front->content->senderName);
	printf("%s\n", testQueue->front->content->messageContent);
	printf("%s\n", testQueue->front->next->content->senderName);
 	printf("%s\n", testQueue->front->next->content->messageContent);
	printf("%s\n", testQueue->front->next->next->content->senderName);
	printf("%s\n", testQueue->front->next->next->content->messageContent);
	printf("%s\n", testQueue->front->content->senderName);
	dequeueOutputMessage(testQueue);
	printf("%s\n", testQueue->front->content->senderName);
	dequeueOutputMessage(testQueue);
	printf("%s\n", testQueue->front->content->senderName);
//	dequeueOutputMessage(testQueue);
//	printf("%s\n", testQueue->front->content->senderName);
	return 0;
}
