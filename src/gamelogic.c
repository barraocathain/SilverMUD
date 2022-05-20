// gamelogic.c: Contains function definitons for dealing with the game's logic.
// Barry Kane, 2022.
#include <stdio.h>
#include <string.h>
#include "../include/constants.h"
#include "../include/gamelogic.h"
#include "../include/playerdata.h"
#include "../include/inputoutput.h"

// =======================
// -=[ Main Game Loop ]=-:
// =======================

// Thread function which runs the main game loop, given the needed parameters:
void * gameLogicLoop(void * parameters)
{
	char formattedString[64];
	gameLogicParameters * threadParameters = parameters;
	inputMessage * currentInput = NULL;
	bool keepRunning = true;
	while(keepRunning)
	{
		// Check for new messages and pop them off the queue:
		if(threadParameters->inputQueue->currentLength != 0)
		{
			while(threadParameters->inputQueue->lock == true)
			{
				threadParameters->inputQueue->lock = true;
			}
			currentInput = peekInputMessage(threadParameters->inputQueue);
			userInputSanatize(currentInput->content->messageContent, MAX);
			// A slash as the first character means the message is a user command:
			if(currentInput->content->messageContent[0] == '/')
			{
				// TODO: Implement Command Queue.
				// For now, basic intepretation will do.

				// Exit command: Sends an "empty" exit message to disconnect a client:
				if(strncmp(&currentInput->content->messageContent[1], "EXIT", 4) == 0 ||
				   strncmp(&currentInput->content->messageContent[1], "exit", 4) == 0) 
				{
					userMessage * exitMessage = malloc(sizeof(userMessage));
					exitMessage->senderName[0] = '\0';
					exitMessage->messageContent[0] = '\0';
					queueTargetedOutputMessage(threadParameters->outputQueue, exitMessage, &currentInput->sender, 1);
					free(exitMessage);
				}

				// Move command: Moves the current player down a path in their current area, given a pathname or number:
				if(strncmp(&currentInput->content->messageContent[1], "MOVE", 4) == 0 ||
				   strncmp(&currentInput->content->messageContent[1], "move", 4) == 0)
				{
					userMessage * moveMessage = malloc(sizeof(userMessage));
					bzero(moveMessage, sizeof(userMessage));
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
						moveMessage->senderName[0] = '\0';
						strncat(moveMessage->messageContent, currentInput->sender->currentArea->areaName, 33);
						strncat(moveMessage->messageContent, "\n", 2); 
						strncat(moveMessage->messageContent, currentInput->sender->currentArea->areaDescription, 256);
						strncat(moveMessage->messageContent, "\nYou can go:", 13);		
						for(int index = 0; index < 16; index++)
						{								
							if(currentInput->sender->currentArea->areaExits[index] != NULL)
							{
								snprintf(formattedString, 64, "\n\t%d. %s", index + 1, currentInput->sender->currentArea->areaExits[index]->pathName);
								strncat(moveMessage->messageContent, formattedString, 64);
							}
						}
						queueTargetedOutputMessage(threadParameters->outputQueue, moveMessage, &currentInput->sender, 1);
					}
					free(moveMessage);
				}
				
				// Look command: Sends the current area's name, description, and 
				if(strncmp(&currentInput->content->messageContent[1], "LOOK", 4) == 0 ||
				   strncmp(&currentInput->content->messageContent[1], "look", 4) == 0)
				{
					userMessage * lookMessage = malloc(sizeof(userMessage));
					strncat(lookMessage->messageContent, currentInput->sender->currentArea->areaName, 33);
					strncat(lookMessage->messageContent, "\n", 2); 
					strncat(lookMessage->messageContent, currentInput->sender->currentArea->areaDescription, 256);
					strncat(lookMessage->messageContent, "\nYou can go:", 13);		
					for(int index = 0; index < 16; index++)
					{								
						if(currentInput->sender->currentArea->areaExits[index] != NULL)
						{
							snprintf(formattedString, 64, "\n\t%d. %s", index + 1, currentInput->sender->currentArea->areaExits[index]->pathName);
							strncat(lookMessage->messageContent, formattedString, 64);
						}
					}
					queueTargetedOutputMessage(threadParameters->outputQueue, lookMessage, &currentInput->sender, 1);
					free(lookMessage);
				}
				
				// Name command: Checks if the name is isn't used and is valid, then changes the player's name:
				if(strncmp(&currentInput->content->messageContent[1], "NAME", 4) == 0 ||
				   strncmp(&currentInput->content->messageContent[1], "name", 4) == 0)
				{
					
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
