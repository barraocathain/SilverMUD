// gamelogic.c: Contains function definitons for dealing with the game's logic.
// Barry Kane, 2022.
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "queue.h"
#include "constants.h"
#include "gamelogic.h"
#include "playerdata.h"
#include "linkedlist.h"
#include "inputoutput.h"

// =======================
// -=[ Main Game Loop ]=-:
// =======================

// Thread function which runs the main game loop, given the needed parameters:
void * gameLogicHandler(void * parameters)
{
	gameLogicParameters * threadParameters = parameters;
	inputMessage * currentInput = NULL;
	queue * commandQueue = createQueue();
	while (true)
	{
		// Evaluate remaining commands:
		while (commandQueue->itemCount != 0)
		{
			evaluateNextCommand(threadParameters, commandQueue);
		}

		// Wait if there is nothing to do:
		if (threadParameters->inputQueue->itemCount == 0)
		{
			pthread_cond_wait(&threadParameters->inputQueue->condition, &threadParameters->inputQueue->mutex);
		}
		
		// Check for new messages and pop them off the queue:
		if (threadParameters->inputQueue->itemCount != 0)
		{
			while (threadParameters->inputQueue->lock == true);
			threadParameters->inputQueue->lock = true;
			currentInput = peekQueue(threadParameters->inputQueue)->data.inputMessage;
			userInputSanatize(currentInput->content->messageContent, MAX);
			// A slash as the first character means the message is a user command:
			if (currentInput->content->messageContent[0] == '/')
			{
				queueMessagedCommand(commandQueue, currentInput);
			}

			else if (!(currentInput->sender->currentArea == getFromList(threadParameters->areaList, 0)->area) &&
					 currentInput->content->messageContent[0] != '\n')
			{
				// Copy the correct name into the sender name field:
				strncpy(currentInput->content->senderName, currentInput->sender->playerName, 32);
				currentInput->content->senderName[31] = '\0';

				if(currentInput->sender->talkingWith == NULL)
				{
					// Allocate an array of playerInfo to store the current players in the area for the output message:
					playerInfo ** recipients = calloc(PLAYERCOUNT, sizeof(playerInfo*));

					// Initialize them all to NULL:
					for (int index = 0; index < PLAYERCOUNT; index++)
					{
						recipients[index] = NULL;
					}

					// Get the players in the current area and add them to our array:
					int recipientIndex = 0;
					for (int playerIndex = 0; playerIndex < *threadParameters->playerCount; playerIndex++)
					{
						if (threadParameters->connectedPlayers[playerIndex].currentArea == currentInput->sender->currentArea)
						{
							recipients[recipientIndex] = &threadParameters->connectedPlayers[playerIndex];
							recipientIndex++;
						}
					}

					// Create the outputMessage for the queue:
					outputMessage * newOutputMessage = createTargetedOutputMessage(currentInput->content, recipients, recipientIndex);
					
					// Push the message onto the queue:
					pushQueue(threadParameters->outputQueue, newOutputMessage, OUTPUT_MESSAGE);
				
					// Free the array;
					free(recipients);
				}
				else
				{
					// Allocate an array of one playerInfo to store the pointer to the other player in the conversation:
					playerInfo ** recipients = calloc(1, (sizeof(playerInfo*)));

					// Set the talkingWith player as the recipient of the message:
					recipients[0] = currentInput->sender->talkingWith;

					// There's only one recipient:
					int recipientIndex = 1;

					// Create the outputMessage for the queue:
					outputMessage * newOutputMessage = createTargetedOutputMessage(currentInput->content, recipients, recipientIndex);

					// Push the message onto the queue:
					pushQueue(threadParameters->outputQueue, newOutputMessage, OUTPUT_MESSAGE);
				
					// Free the array;
					free(recipients);
				}
			}
			bzero(currentInput, sizeof(inputMessage));
			currentInput = NULL;
			threadParameters->inputQueue->lock = false;
			popQueue(threadParameters->inputQueue);
		}
	}
	pthread_exit(NULL);
}

// Evaluate the next commandEvent in a queue:
void queueMessagedCommand(queue * queue, inputMessage * messageToQueue)
{
	// Prepare the new commandEvent:
	commandEvent * newCommand = calloc(1, sizeof(commandEvent));
	newCommand->command = calloc(16, sizeof(char));
	newCommand->arguments = calloc(MAX, sizeof(char));
	newCommand->caller = messageToQueue->sender;
	
	// Seperate the command from it's arguments:
	strtok(messageToQueue->content->messageContent, " ");
	
	// Copy the command and arguments to the new commandEvent:
	memcpy(newCommand->command, &messageToQueue->content->messageContent[1], 16);
	memcpy(newCommand->arguments, &messageToQueue->content->messageContent[strlen(newCommand->command) + 2],
		   MAX - (strlen(newCommand->command) + 2));


	// Ensure the arguments are safe to parse, without adding newlines:
	userNameSanatize(newCommand->command, 16);
	newCommand->command[15] = '\0';
	
	userNameSanatize(newCommand->arguments, MAX);
	newCommand->arguments[MAX - 1] = '\0';

	// Lowercase the command for easier comparison:
	for (char * character = newCommand->command; *character; ++character)
	{
		*character = tolower(*character);
	}
	
	pushQueue(queue, newCommand, COMMAND);
}

// Enqueue a command to a queue:
void queueCommand(queue * queue, char * command, char * arguments, int commandLength, int argumentsLength,
				  playerInfo * callingPlayer)
{
	// Prepare the new commandEvent:
	commandEvent * newCommand = calloc(1, sizeof(commandEvent));
	newCommand->command = calloc(16, sizeof(char));
	newCommand->arguments = calloc(MAX, sizeof(char));
	newCommand->caller = callingPlayer;

	// Copy the command and arguments:
	strncpy(newCommand->command, command, commandLength);
	if (argumentsLength > 0)
	{
		strncpy(newCommand->arguments, arguments, argumentsLength);
	}
	// Ensure the arguments are safe to parse, without adding newlines:
	userNameSanatize(newCommand->command, 16);

	pushQueue(queue, newCommand, COMMAND);
}

// Evaluate the next commandEvent in a queue:
int evaluateNextCommand(gameLogicParameters * parameters, queue * queue)
{
	commandEvent * currentCommand = peekQueue(queue)->data.command;
	while (queue->lock);
	queue->lock = true;	
	if (currentCommand == NULL)
	{
		return -1;
	}

	// Switch to the relevant command based on the hash:
	switch (hashCommand(currentCommand->command, strlen(currentCommand->command)))
	{
		// Look command: Returns the description of the current area and paths:
		case 5626697:
		{
			char formattedString[64];
			userMessage * lookMessage = calloc(1, sizeof(userMessage));
			lookMessage->senderName[0] = '\0';
			strncat(lookMessage->messageContent, currentCommand->caller->currentArea->areaName, 33);
			strncat(lookMessage->messageContent, "\n", 2);
			strncat(lookMessage->messageContent, currentCommand->caller->currentArea->areaDescription, MAX - 35);

			// Allocate an outputMessage for the queue:
			outputMessage * lookOutputMessage = createTargetedOutputMessage(lookMessage, &currentCommand->caller, 1);

			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, lookOutputMessage, OUTPUT_MESSAGE);

			//queueTargetedOutputMessage(parameters->outputQueue, lookMessage, &currentCommand->caller, 1);
			bzero(lookMessage, sizeof(userMessage));

			// Loop through the paths and send the appropriate amount of messages:
			int charCount = 13;
			strncat(lookMessage->messageContent, "You can go:", 13);
		
			if (currentCommand->caller->currentArea->pathList->itemCount > 0)
			{
				for(size_t index = 0; index < currentCommand->caller->currentArea->pathList->itemCount; index++)
				{
					if ((charCount + 64) >= MAX)
					{
						lookOutputMessage = createTargetedOutputMessage(lookMessage, &currentCommand->caller, 1);

						// Queue the outputMessage:
						pushQueue(parameters->outputQueue, lookOutputMessage, OUTPUT_MESSAGE);

						bzero(lookMessage, sizeof(userMessage));
						charCount = 0;
					}
					snprintf(formattedString, 64, "\n\t%ld. %s", index + 1,
							 getFromList(currentCommand->caller->currentArea->pathList, index)->path->pathName);
					strncat(lookMessage->messageContent, formattedString, 64);
					charCount += 64;
				}	   
				// Allocate another outputMessage for the queue:
				lookOutputMessage = createTargetedOutputMessage(lookMessage, &currentCommand->caller, 1);
			
				// Queue the outputMessage:
				pushQueue(parameters->outputQueue, lookOutputMessage, OUTPUT_MESSAGE);
			}

			// Clear the message:
			memset(lookMessage, 0, sizeof(userMessage));
			if(currentCommand->caller->currentArea != getFromList(parameters->areaList, 0)->area)
			{
				// Show the players in the area:
				charCount = 23;
				strncat(lookMessage->messageContent, "These players are here:", 24);
			
				int playerNumber = 1;
				for(int index = 0; index < *(parameters->playerCount); index++)
				{
					if (parameters->connectedPlayers[index].currentArea == currentCommand->caller->currentArea)
					{
						if ((charCount + 38) >= MAX)
						{
							lookOutputMessage = createTargetedOutputMessage(lookMessage, &currentCommand->caller, 1);
						
							// Queue the outputMessage:
							pushQueue(parameters->outputQueue, lookOutputMessage, OUTPUT_MESSAGE);
							memset(lookMessage, 0, sizeof(userMessage));
							charCount = 0;
						}
						snprintf(formattedString, 38, "\n%02d. %32s", playerNumber++,
								 parameters->connectedPlayers[index].playerName);
						strncat(lookMessage->messageContent, formattedString, 37);
						charCount += 38;

						// Allocate another outputMessage for the queue:
						lookOutputMessage = createTargetedOutputMessage(lookMessage, &currentCommand->caller, 1);
						
						// Queue the outputMessage:
						pushQueue(parameters->outputQueue, lookOutputMessage, OUTPUT_MESSAGE);
					}
				}
			}
			
			free(lookMessage);
			break;
		}
		
		// Stat command: Displays the current character's sheet.
		case 5987604:
		{
			char * formattedString = calloc(121, sizeof(char));
			userMessage * statMessage = calloc(1, sizeof(userMessage));
			statMessage->senderName[0] = '\0';
			// Basic status: Name, level, location.
			snprintf(formattedString, 120, "%s, Level %d | %s\n", currentCommand->caller->playerName,
					 currentCommand->caller->stats->level, currentCommand->caller->currentArea->areaName);
			strncat(statMessage->messageContent, formattedString, 120);

			// Current stats: Health and WISED.
			snprintf(formattedString, 120,
					 "Health: %d/%d\nStats:\n\tWits: %2d | Intellect: %2d | Strength: %2d | Endurance: %2d | Dexerity: %2d \n",
					 currentCommand->caller->stats->currentHealth, currentCommand->caller->stats->maxHealth,
					 currentCommand->caller->stats->wits, currentCommand->caller->stats->intellect,
					 currentCommand->caller->stats->strength, currentCommand->caller->stats->endurance,
					 currentCommand->caller->stats->dexerity);
			strncat(statMessage->messageContent, formattedString, 120);

			// Levelling stats: Current XP, and spec points.
			if (currentCommand->caller->stats->specPoints > 0 || currentCommand->caller->stats->skillPoints > 0)
			{
				snprintf(formattedString, 120, "Current Experience: %ld | Spec Points Available: %d | Skill Points Available: %d",
						 currentCommand->caller->stats->experience, currentCommand->caller->stats->specPoints, currentCommand->caller->stats->skillPoints);
			}
			else
			{
				snprintf(formattedString, 120, "Current Experience: %ld", currentCommand->caller->stats->experience);
			}
			strncat(statMessage->messageContent, formattedString, 120);

			// Allocate an outputMessage for the queue:
			outputMessage * statOutputMessage = createTargetedOutputMessage(statMessage, &currentCommand->caller, 1);

			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, statOutputMessage, OUTPUT_MESSAGE);
		
			bzero(statMessage->messageContent, sizeof(char) * MAX);
			if (currentCommand->caller->skills->head != NULL)
			{			
				size_t skillIndex = 0;
				int charCount = 0;
				bool addNewline = false;
				playerSkill * skill;
				while (skillIndex < currentCommand->caller->skills->itemCount)
				{
					skill = getFromList(currentCommand->caller->skills, skillIndex)->skill;
					skillIndex++;
					snprintf(formattedString, 120, "| %2d | %31s ", skill->skillPoints, skill->skillName);
					charCount += 43;
					strncat(statMessage->messageContent, formattedString, 120);
					if ((charCount + 43) >= MAX)
					{
						// Allocate an outputMessage for the queue:
						statOutputMessage = createTargetedOutputMessage(statMessage, &currentCommand->caller, 1);

						// Queue the outputMessage:
						pushQueue(parameters->outputQueue, statOutputMessage, OUTPUT_MESSAGE);
						bzero(statMessage, sizeof(userMessage));
						charCount = 0;
						break;
					}
					else if (addNewline)
					{
						strncat(statMessage->messageContent, "|\n", 3);
						charCount++;
						addNewline = false;
					}
					else
					{
						addNewline = true;
					}		  
				}
				// Allocate an outputMessage for the queue:
				statOutputMessage = createTargetedOutputMessage(statMessage, &currentCommand->caller, 1);

				// Queue the outputMessage:
				pushQueue(parameters->outputQueue, statOutputMessage, OUTPUT_MESSAGE);
			}
			free(statMessage);
			free(formattedString);

			break;
		}
		
		// Spec command: Assign spec points to stats:
		case 5982259:
		{
			userMessage * specMessage = calloc(1, sizeof(userMessage));
			specMessage->senderName[0] = '\0';
			char * formattedString = calloc(121, sizeof(char));
			if (currentCommand->caller->stats->specPoints > 0)
			{
				int selectedAmount = 0;
				strtok(currentCommand->arguments, " ");
				selectedAmount = atoi(&currentCommand->arguments[strlen(currentCommand->arguments) + 1]);
				coreStat selectedStat = getCoreStatFromString(currentCommand->arguments, 16);
				if (selectedAmount > 0 && (currentCommand->caller->stats->specPoints - selectedAmount) >= 0)
				{
					switch (selectedStat)
					{
						case WITS:
						{
							currentCommand->caller->stats->wits += selectedAmount;
							strncat(specMessage->messageContent, "Increased wits.", 16);
							currentCommand->caller->stats->specPoints -= selectedAmount;
							break;
						}
						case INTELLECT:
						{
							currentCommand->caller->stats->intellect += selectedAmount;
							strncat(specMessage->messageContent, "Increased intellect.", 21);
							currentCommand->caller->stats->specPoints -= selectedAmount;
							break;
						}
						case STRENGTH:
						{
							currentCommand->caller->stats->strength += selectedAmount;
							strncat(specMessage->messageContent, "Increased strength.", 20);
							currentCommand->caller->stats->specPoints -= selectedAmount;
							break;
						}
						case ENDURANCE:
						{
							currentCommand->caller->stats->endurance += selectedAmount;
							strncat(specMessage->messageContent, "Increased endurance.", 21);
							currentCommand->caller->stats->specPoints -= selectedAmount;
							break;
						}
						case DEXERITY:
						{
							currentCommand->caller->stats->dexerity += selectedAmount;
							strncat(specMessage->messageContent, "Increased dexerity.", 21);
							currentCommand->caller->stats->specPoints -= selectedAmount;
							break;
						}
						case INVALID:
						{
							strncat(specMessage->messageContent, "Invalid stat.", 21);
						}
					}
				}
				else
				{
					strncat(specMessage->messageContent, "You have entered an invalid amount of spec points.", 51);			
				}
			}
			else
			{
				strncat(specMessage->messageContent, "You have no spec points available.", 35);			
			}
		
			// Allocate an outputMessage for the queue:
			outputMessage * specOutputMessage = createTargetedOutputMessage(specMessage, &currentCommand->caller, 1);

			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, specOutputMessage, OUTPUT_MESSAGE);

			// Show the new stat sheet:
			queue->lock = false;
			queueCommand(queue, "stat", "", 5, 0, currentCommand->caller);
			queue->lock = true;
		
			// Free the finished message:
			free(specMessage);
			free(formattedString);

			break;
		}
		
		// Try command: Attempt to use a stat or skill on an object:
		case 163143:
		{
			// Allocate the userMessage to send:
			userMessage * tryMessage = calloc(1, (sizeof(userMessage)));
			tryMessage->senderName[0] = '\0';

			// Temporary message until we can implement objects, events, and challenges.
			strcpy(tryMessage->messageContent, "The try command is currently not implemented. Implement it if you want to use it.\n");
		
			// Allocate an outputMessage for the queue:
			outputMessage * tryOutputMessage = createTargetedOutputMessage(tryMessage, &currentCommand->caller, 1);

			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, tryOutputMessage, OUTPUT_MESSAGE);
			
			// Free the userMessage:
			free(tryMessage);
			
			break;
		}
		
		// Move command: Moves the caller to a different area given a path name or number:
		case 5677603:
		{
			char requestedPath[32];
			if (strlen(currentCommand->arguments) > 0 && currentCommand->caller->currentArea != getFromList(parameters->areaList, 0)->area)
			{
				memcpy(requestedPath, currentCommand->arguments, 32);
				userNameSanatize(requestedPath, 32);
				requestedPath[31] = '\0';
				if (movePlayerToArea(currentCommand->caller, requestedPath) == 0)
				{
					// Call the look command after moving. It's fine to unlock, because the loop won't
					// continue until the command is queued:
					queue->lock = false;
					queueCommand(queue, "look", "", 5, 0, currentCommand->caller);
					queue->lock = true;
				}
			}

			break;
		}

		// Skill command: Allows you to put skill points into skills:
		case 221096235:
		{
			userMessage * skillMessage = calloc(1, sizeof(userMessage));
			skillMessage->senderName[0] = '\0';
			if ((currentCommand->caller->stats->skillPoints - 1) >= 0)
			{
				int returnValue = takeSkill(parameters->globalSkillList, currentCommand->arguments,
											strlen(currentCommand->arguments), currentCommand->caller);
				switch(returnValue)
				{
					case -1:
					{
						strcpy(skillMessage->messageContent, "Not a valid skill.");
						break;
					}
					case 0:
					{
						strcpy(skillMessage->messageContent, "Took ");
						strcat(skillMessage->messageContent, currentCommand->arguments);
						strcat(skillMessage->messageContent, ".");					
						currentCommand->caller->stats->skillPoints--;
						break;
					}
				}
			}
			else
			{
				strcpy(skillMessage->messageContent, "You don't have enough skill points to take this skill.\n");
			}

			// Allocate an outputMessage for the queue:
			outputMessage * skillOutputMessage = createTargetedOutputMessage(skillMessage, &currentCommand->caller, 1);
				
			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, skillOutputMessage, OUTPUT_MESSAGE);
		
			free(skillMessage);
			break;
		}

		// Listskills commands: List all available skills on the server:
		case 2395990522:
		{
			userMessage * listMessage = calloc(1, sizeof(userMessage));
			char * formattedString = calloc(121, sizeof(char));
			int charCount = 0;
			size_t skillIndex = 0;
			bool addNewline = false;
			playerSkill * currentSkill;
			while (skillIndex < parameters->globalSkillList->itemCount)
			{
				currentSkill = getFromList(parameters->globalSkillList, skillIndex)->skill;
				snprintf(formattedString, 120, "| %-31s ", currentSkill->skillName);
				charCount += 43;
				strncat(listMessage->messageContent, formattedString, 120);
				if ((charCount + 46) >= MAX)
				{
					// Allocate an outputMessage for the queue:
					outputMessage * listOutputMessage = createTargetedOutputMessage(listMessage, &currentCommand->caller, 1);
				
					// Queue the outputMessage:
					pushQueue(parameters->outputQueue, listOutputMessage, OUTPUT_MESSAGE);
				
					bzero(listMessage, sizeof(userMessage));
					charCount = 0;
					addNewline = false;
				}
				else if (addNewline)
				{
					strncat(listMessage->messageContent, "|\n", 3);
					charCount++;
					addNewline = false;
				}
				else
				{
					addNewline = true;
				}
				skillIndex++;
			}
			// Allocate an outputMessage for the queue:
			outputMessage * listOutputMessage = createTargetedOutputMessage(listMessage, &currentCommand->caller, 1);
				
			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, listOutputMessage, OUTPUT_MESSAGE);
			free(listMessage);
			free(formattedString);
			break;
		}
		// Shout command: Allows the player to talk to everyone in the area if they are in a conversation.
		case 220952831:
		{
			// Allocate an array of playerInfo to store the current players in the area for the output message:
			playerInfo ** recipients = calloc(PLAYERCOUNT, sizeof(playerInfo*));
			
			// Initialize them all to NULL:
			for (int index = 0; index < PLAYERCOUNT; index++)
			{
				recipients[index] = NULL;
			}
			
			// Get the players in the current area and add them to our array:
			int recipientIndex = 0;
			for (int playerIndex = 0; playerIndex < *parameters->playerCount; playerIndex++)
			{
				if (parameters->connectedPlayers[playerIndex].currentArea == currentCommand->caller->currentArea)
				{
					recipients[recipientIndex] = &parameters->connectedPlayers[playerIndex];
					recipientIndex++;
				}
			}

			// Create a userMessage to be filled with the data from the command's arguments and caller:
			userMessage * shoutMessage = calloc(1, sizeof(userMessage));

			// Copy in the data and terminate it:
			strncpy(shoutMessage->senderName, currentCommand->caller->playerName, 32);
			shoutMessage->senderName[31] = '\0';
			strncpy(shoutMessage->messageContent, currentCommand->arguments, MAX);
			shoutMessage->messageContent[MAX - 1] = '\0';
			strncat(shoutMessage->messageContent, "\n", MAX);
			
			// Create the outputMessage for the queue:
			outputMessage * shoutOutputMessage = createTargetedOutputMessage(shoutMessage, recipients, recipientIndex);

			// Push the message onto the output queue:
			pushQueue(parameters->outputQueue, shoutOutputMessage, OUTPUT_MESSAGE);
			
			// Free the array:
			free(recipients);

			break;
		}
		// Talk command: Allows the player to begin a chat session with another player:
		case 6012644:
		{
			userMessage * talkMessage = calloc(1, sizeof(userMessage));
			talkMessage->senderName[0] = '\0';

			// If there's no name specified, end the current chat sessions.
			if (currentCommand->arguments[0] == '\0' || currentCommand->arguments == NULL)
			{
				currentCommand->caller->talkingWith = NULL;
				strcpy(talkMessage->messageContent, "Conversation ended.");
			}
			else
			{
				for(int playerIndex = 0; playerIndex < *parameters->playerCount; playerIndex++)
				{
					if(strncmp(currentCommand->arguments, parameters->connectedPlayers[playerIndex].playerName, 31) == 0)
					{
						currentCommand->caller->talkingWith = &(parameters->connectedPlayers[playerIndex]);

						// Fill out the message to inform the receiving user what is happening:
						strncpy(talkMessage->messageContent, currentCommand->caller->playerName, 31);
						strcat(talkMessage->messageContent, " is talking to you.");

						playerInfo ** recipients = calloc(1, (sizeof(playerInfo*)));
						recipients[0] = &(parameters->connectedPlayers[playerIndex]);
						
						// Allocate an outputMessage for the receiving user:
						outputMessage * talkReceiverMessage = createTargetedOutputMessage(talkMessage, recipients, 1);

						// Queue the outputMessage:
						pushQueue(parameters->outputQueue, talkReceiverMessage, OUTPUT_MESSAGE);

						// Prep the message to the calling user.
						strcpy(talkMessage->messageContent, "Conversation begun with: ");
						strcat(talkMessage->messageContent, parameters->connectedPlayers[playerIndex].playerName);
					}
				}
					
			}
			
			// Allocate an outputMessage for the queue:
			outputMessage * talkOutputMessage = createTargetedOutputMessage(talkMessage, &currentCommand->caller, 1);

			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, talkOutputMessage, OUTPUT_MESSAGE);

			// Free the userMessage:
			free(talkMessage);

			break;
		}
		
		// Exit command: Sends an "empty" exit message to disconnect a client:
		case 5284234:
		{
			// Allocate a userMessage containing null characters as the first char in both fields:
			userMessage * exitMessage = calloc(1, (sizeof(userMessage)));
			exitMessage->senderName[0] = '\0';
			exitMessage->messageContent[0] = '\0';

			// Allocate an outputMessage for the queue:
			outputMessage * exitOutputMessage = createTargetedOutputMessage(exitMessage, &currentCommand->caller, 1);
			
			// Queue the outputMessage:
			pushQueue(parameters->outputQueue, exitOutputMessage, OUTPUT_MESSAGE);
			
			// Free the userMessage
			free(exitMessage);

			break;
		}
		
		// Join command: Allows the player to join the game given a name:
		// TODO: Implement login/character creation. Will be a while:
		case 5525172:
		{
			if (currentCommand->caller->currentArea == getFromList(parameters->areaList, 0)->area)
			{
				bool validName = true;
				for(int index = 0; index < *parameters->playerCount; index++)
				{
					if (currentCommand->arguments[0] == '\0')
					{
						validName = false;
					}
					if (strncmp(currentCommand->arguments, parameters->connectedPlayers[index].playerName, 16) == 0)
					{
						validName = false;
					}
				}
				if (validName)
				{
					strncpy(currentCommand->caller->playerName, currentCommand->arguments, 16);
					currentCommand->caller->currentArea = getFromList(parameters->areaList, 1)->area;
					// Call the look command after joining. It's fine to unlock, because the loop won't
					// continue until the command is queued:
					queue->lock = false;
					queueCommand(queue, "look", "", 5, 0, currentCommand->caller);
					queue->lock = true;
				}
			}
			break;
		}
	}

	// Remove the current command and unlock the queue:
	bzero(currentCommand->command, sizeof(char) * 16);
	bzero(currentCommand->arguments, sizeof(char) * MAX);
	currentCommand = NULL;
	queue->lock = false;	
	popQueue(queue);
	return 0;
}

// Run a stat check for the given player, returning an outcome:
outcome statCheck(playerInfo * player, int chance, coreStat statToCheck)
{
	// Calculate the chance:
	if (chance > 100 || chance < 0)
	{
		return ERROR;
	}
	chance = 100 - chance;

	// Calculate the modifier:
	int modifier = 0;
	switch(statToCheck)
	{
		case WITS:
		{
			modifier = player->stats->wits * 4;
			break;
		}
		case INTELLECT:
		{
			modifier = player->stats->intellect * 4;
			break;
		}
		case STRENGTH:
		{
			modifier = player->stats->strength * 4;
			break;
		}
		case ENDURANCE:
		{
			modifier = player->stats->endurance * 4;
			break;
		}
		case DEXERITY:
		{
			modifier = player->stats->dexerity * 4;
			break;
		}
		default:
		{
			return ERROR;
		}
	}
	int attempt = (random() % 100) + modifier;
	if (attempt >= chance)
	{
		if (attempt >= 98)
		{
			return CRITICAL_SUCCESS;
		}
		else
		{
			return SUCCESS;
		}
	}
	else
	{
		if (attempt <= 2)
		{
			return CRITICAL_FAILURE;
		}
		else
		{
			return FAILURE;
		}
	}
}

// Run a skill check for the given player, returning an outcome:
outcome skillCheck(playerInfo * player, int chance, char * skillName, size_t skillNameLength, list * globalSkillList)
{
	// Calculate the chance:
	if (chance > 100 || chance < 0)
	{
		return ERROR;
	}
	chance = 100 - chance;

	// Check if the player has the given skill:
	bool playerHasSkill = false;
	size_t playerIndex = 0;
	while (playerIndex < player->skills->itemCount)
	{
		if (strncmp(skillName, getFromList(player->skills, playerIndex)->skill->skillName, skillNameLength) != 0)
		{
			playerHasSkill = true;
			break;
		}
		playerIndex++;
	}

	// If the player doesn't have the skill, check if it's in the game and is trained:
	bool trainedSkill = false;
	size_t globalIndex = 0;
	while (globalIndex < globalSkillList->itemCount)
	{
		if (strncmp(skillName, getFromList(globalSkillList, globalIndex)->skill->skillName, skillNameLength) != 0)
		{
			trainedSkill = getFromList(globalSkillList, globalIndex)->skill->trainedSkill;
			break;
		}
		globalIndex++;
	}
	
	// Calculate the modifier:
	int modifier = 0;
	if (trainedSkill)
	{
		modifier = -100;
	}
	else if (playerHasSkill)
	{
		modifier = getFromList(player->skills, playerIndex)->skill->skillModifier * 4;
	}
	
	// Attempt the check:
	int attempt = (random() % 100) + modifier;
	if (attempt >= chance)
	{
		if (attempt >= 98)
		{
			return CRITICAL_SUCCESS;
		}
		else
		{
			return SUCCESS;
		}
	}
	else
	{
		if (attempt <= 2)
		{
			return CRITICAL_FAILURE;
		}
		else
		{
			return FAILURE;
		}
	}
}

// Move a player along a path in their current area:
int movePlayerToArea(playerInfo * player, char * requestedPath)
{
	// Check if a number was given first:
	size_t selected = atoi(requestedPath);
	if (selected != 0 && !(selected > player->currentArea->pathList->itemCount))
	{
		if (getFromList(player->currentArea->pathList, selected - 1)->path != NULL &&
			getFromList(player->currentArea->pathList, selected - 1)->path->areaToJoin != NULL)
		{
			player->currentArea = getFromList(player->currentArea->pathList, selected - 1)->path->areaToJoin;
			return 0;
		}
		else
		{
			return 1;
		}
	}

	// Otherwise search for the description:
	for (size_t index = 0; index < player->currentArea->pathList->itemCount; index++)
	{
		if (strncmp(getFromList(player->currentArea->pathList, index)->path->pathName,
					requestedPath, 32) == 0)
		{
			player->currentArea = getFromList(player->currentArea->pathList, index)->path->areaToJoin;
			return 0;
		}
	}   
	return 1;
}

// A hash function for distinguishing commands for the game logic handler:
unsigned int hashCommand(char * command, unsigned int commandLength)
{
	unsigned int hash = 0;
	char * currentCharacter = command;
	
	for (unsigned int index = 0; index < commandLength && *currentCharacter != '\0'; currentCharacter++)
	{
		hash = 37 * hash + *currentCharacter;
	}

	return hash;
}
