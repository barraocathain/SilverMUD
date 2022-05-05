// gamelogic.c: Contains function definitons for dealing with the game's logic.
// Barry Kane, 2022.
#include <string.h>
#include "constants.h"
#include "gamelogic.h"
#include "playerdata.h"
#include "inputoutput.h"

// =======================
// -=[ Main Game Loop ]=-:
// =======================

// Thread function which runs the main game loop, given the needed parameters:
void * gameLogicLoop(void * parameters)
{
	gameLogicParameters * threadParameters = parameters;
	inputMessage * currentInput = NULL;
	bool keepRunning = true;
	while(keepRunning)
	{
		if(threadParameters->inputQueue->currentLength != 0)
		{
			while(threadParameters->inputQueue->lock == true)
			{
				threadParameters->inputQueue->lock = true;
			}
			currentInput = peekInputMessage(threadParameters->inputQueue);
			userInputSanatize(currentInput->content->messageContent, MAX);
			if(currentInput->content->messageContent[0] == '/')
			{
				// TODO: Implement Command Queue.
				// For now, basic intepretation will do. 
				if(strncmp(&currentInput->content->messageContent[1], "EXIT", 4) == 0) 
				{
					userMessage * exitMessage = malloc(sizeof(userMessage));
					exitMessage->senderName[0] = '\0';
					exitMessage->messageContent[0] = '\0';
					queueTargetedOutputMessage(threadParameters->outputQueue, exitMessage, &currentInput->sender, 1);
					free(exitMessage);
				}
				if(strncmp(&currentInput->content->messageContent[1], "MOVE", 4) == 0)
				{
					userMessage * moveMessage = malloc(sizeof(userMessage));
					char requestedPath[32];
					strncpy(requestedPath, &currentInput->content->messageContent[6], 32);
					userInputSanatize(requestedPath, 32);
					// Remove newlines:
					for (int index = 0; index < 32; index++)
					{
						if (requestedPath[index] == '\n')
						{
							requestedPath[index] = '\0';
						}
					}
					requestedPath[31] = '\0';
					if(movePlayerToArea(currentInput->sender, requestedPath) == 0)
					{
						strcpy(moveMessage->senderName, "\0");
						strcpy(moveMessage->messageContent, currentInput->sender->currentArea->areaDescription);
						queueTargetedOutputMessage(threadParameters->outputQueue, moveMessage, &currentInput->sender, 1);
					}
					free(moveMessage);
				}
			}
			else
			{
				strncpy(currentInput->content->senderName, currentInput->sender->playerName, 32);
				// Create an array of players in the same area to receive the message:
				playerInfo ** recipients = malloc(sizeof(playerInfo*) * *threadParameters->playerCount);
				for(int index = 0; index < *threadParameters->playerCount; index++)
				{
					recipients[index] = NULL;
				}
				int recipientCount = 0;
				for(int playerIndex = 0; playerIndex < *threadParameters->playerCount; playerIndex++)
				{
					if(threadParameters->connectedPlayers[playerIndex].currentArea == currentInput->sender->currentArea)
					{
						recipients[recipientCount] = &threadParameters->connectedPlayers[playerIndex];
						recipientCount++;
					}
				}
				if(currentInput->content->messageContent[0] != '\n')
				{
					queueTargetedOutputMessage(threadParameters->outputQueue, currentInput->content, recipients, recipientCount);
				}
				free(recipients);
			}
			currentInput = NULL;
			threadParameters->inputQueue->lock = false;
			dequeueInputMessage(threadParameters->inputQueue);
		}
	}
	return NULL;
}
