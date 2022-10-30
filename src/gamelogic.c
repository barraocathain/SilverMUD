// gamelogic.c: Contains function definitons for dealing with the game's logic.
// Barry Kane, 2022.
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
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
	commandQueue * commandQueue = createCommandQueue();
	while(true)
	{
		// Evaluate remaining commands:
		if(commandQueue->currentLength != 0)
		{
			evaluateNextCommand(threadParameters, commandQueue);
		}
		// Check for new messages and pop them off the queue:
		if(threadParameters->inputQueue->currentLength != 0)
		{
			while(threadParameters->inputQueue->lock == true);
			threadParameters->inputQueue->lock = true;
			currentInput = peekInputMessage(threadParameters->inputQueue);
			userInputSanatize(currentInput->content->messageContent, MAX);
			// A slash as the first character means the message is a user command:
			if(currentInput->content->messageContent[0] == '/')
			{
				queueMessagedCommand(commandQueue, currentInput);
			}
			else if(currentInput->sender->currentArea == getAreaFromList(threadParameters->areaList, 0))
			{
				currentInput = NULL;
				threadParameters->inputQueue->lock = false;
				dequeueInputMessage(threadParameters->inputQueue);
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
	pthread_exit(NULL);
}

// Create a commandQueue:
commandQueue * createCommandQueue(void)
{
	commandQueue * newCommandQueue = calloc(1, sizeof(commandQueue));
	newCommandQueue->back = NULL;
	newCommandQueue->front = NULL;
	newCommandQueue->lock = false;
	newCommandQueue->paused = false;
	newCommandQueue->currentLength = 0;
	return newCommandQueue;
}

// Return the front commandEvent from a commandQueue:
commandEvent * peekCommand(commandQueue * queue)
{
	// Do nothing until the command queue is unlocked.
	while(queue->lock);

	// Return the front item.
	return queue->front;
}

// Enqueue a messaged command to a commandQueue:
int queueMessagedCommand(commandQueue * queue, inputMessage * messageToQueue)
{
	// Prepare the new commandEvent:
	commandEvent * newCommand = calloc(1, sizeof(commandEvent));
	newCommand->command = calloc(16, sizeof(char));
	newCommand->arguments = calloc(MAX, sizeof(char));
	newCommand->caller = messageToQueue->sender;
	
	// Seperate the command from it's arguments:
	strtok(messageToQueue->content->messageContent, " ");

	// Copy the command and arguments to the new commandEvent:
	strncpy(newCommand->command, &messageToQueue->content->messageContent[1], 16);
	strncpy(newCommand->arguments, &messageToQueue->content->messageContent[strlen(newCommand->command) + 2],
			MAX - (strlen(newCommand->command) + 2));

	// Ensure the arguments are safe to parse, without adding newlines:
	userNameSanatize(newCommand->command, 16);
	userNameSanatize(newCommand->arguments, MAX);

	// Lowercase the command for easier comparison:
	for (char * character = newCommand->command; *character; ++character)
	{
		*character = tolower(*character);
	}
	
	// Wait for the queue to unlock:
	while (queue->lock);

	// Check that we're not overflowing the queue:
	if ((queue->currentLength + 1) > MAXQUEUELENGTH)
	{
		// Unlock the queue:
		queue->lock = false;
		return -1;
	}
	else
	{
		// If the queue is empty, set the first commandEvent as both the front and back of the queue:
		if(queue->front == NULL)
		{
			queue->front = newCommand;
			queue->back = newCommand;
			queue->currentLength++;
			
			// Unlock the queue:
			queue->lock = false;

			return 0;
		}
		else
		{
			queue->back->next = newCommand;
			queue->back = newCommand;
			queue->currentLength++;

            // Unlock the queue:
			queue->lock = false;

			return 0;
		}
	}
}

// Enqueue a command to a commandQueue:
int queueCommand(commandQueue * queue, char * command, char * arguments, int commandLength, int argumentsLength, playerInfo * callingPlayer)
{
	// Prepare the new commandEvent:
	commandEvent * newCommand = calloc(1, sizeof(commandEvent));
	newCommand->command = calloc(16, sizeof(char));
	newCommand->arguments = calloc(MAX, sizeof(char));
	newCommand->caller = callingPlayer;

	// Copy the command and arguments:
	strncpy(newCommand->command, command, commandLength);
	strncpy(newCommand->arguments, arguments, argumentsLength);

	// Ensure the arguments are safe to parse, without adding newlines:
	userNameSanatize(newCommand->command, 16);

	// Wait for the queue to unlock:
	while (queue->lock);

	// Check that we're not overflowing the queue:
	if ((queue->currentLength + 1) > MAXQUEUELENGTH)
	{
		// Unlock the queue:
		queue->lock = false;
		return -1;
	}
	else
	{
		// If the queue is empty, set the first commandEvent as both the front and back of the queue:
		if(queue->front == NULL)
		{
			queue->front = newCommand;
			queue->back = newCommand;
			queue->currentLength++;

			// Unlock the queue:
			queue->lock = false;

			return 0;
		}
		else
		{
			queue->back->next = newCommand;
			queue->back = newCommand;
			queue->currentLength++;

			// Unlock the queue:
			queue->lock = false;

			return 0;
		}
	}
}

// Dequeue the front commandEvent from a commandQueue:
int dequeueCommand(commandQueue * queue)
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
		free(queue->front->command);
		free(queue->front->arguments);
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
		commandEvent * commandToDelete = queue->front;
		queue->front = queue->front->next;
		free(commandToDelete->command);
		free(commandToDelete->arguments);
		free(commandToDelete);
		queue->currentLength--;
		queue->lock = false;
		return 0;
	}
}

// Evaluate the next commandEvent:
int evaluateNextCommand(gameLogicParameters * parameters, commandQueue * queue)
{
	commandEvent * currentCommand = peekCommand(queue);
	while(queue->lock);
	queue->lock = true;	
	if(currentCommand == NULL)
	{
		return -1;
	}
	// Try command: Attempt to use a stat or skill on an object:
	if(strncmp(currentCommand->command, "try", 3) == 0)
	{
		userMessage * tryMessage = malloc(sizeof(userMessage));
		tryMessage->senderName[0] = '\0';
		switch (getCoreStatFromString(currentCommand->arguments, 9))
		{
			case STRENGTH:
			{
				switch (statCheck(currentCommand->caller, 20, STRENGTH))
				{
					case CRITICAL_FAILURE:
					{
						strcpy(tryMessage->messageContent, "You weak, puny shit. Bet you don't even lift, bro.\n");
						break;
					}
					case FAILURE:
					{
						strcpy(tryMessage->messageContent, "Come on, bro, you should be able to get this set done.\n");
						break;
					}
					case SUCCESS:
					{
						strcpy(tryMessage->messageContent, "Nice set, bro. Keep it up.\n");
						break;
					}
					case CRITICAL_SUCCESS:
					{
						strcpy(tryMessage->messageContent, "HOLY SHIT, BRO! THAT'S SOME MAD REPS RIGHT THERE!\n");
						break;
					}
					default:
					{
						strcpy(tryMessage->messageContent, "I don't even, bro.\n");
					}
				}
				break;
			}
			default:
			{
				sprintf(tryMessage->messageContent,"%d",
						skillCheck(currentCommand->caller, 10, currentCommand->arguments, strlen(currentCommand->arguments),
								   parameters->globalSkillList));
				break;
			}
		}
		queueTargetedOutputMessage(parameters->outputQueue, tryMessage, &currentCommand->caller, 1);
		free(tryMessage);
	}
	// Exit command: Sends an "empty" exit message to disconnect a client:
	if(strncmp(currentCommand->command, "exit", 4) == 0)
	{
		userMessage * exitMessage = malloc(sizeof(userMessage));
		exitMessage->senderName[0] = '\0';
		exitMessage->messageContent[0] = '\0';
		queueTargetedOutputMessage(parameters->outputQueue, exitMessage, &currentCommand->caller, 1);
		free(exitMessage);
	}

	// Move command: Moves the caller to a different area given a path name or number:
	if(strncmp(currentCommand->command, "move", 4) == 0)
	{
		char requestedPath[32];
		if(strlen(currentCommand->arguments) > 0 && currentCommand->caller->currentArea != getAreaFromList(parameters->areaList, 0))
		{
			strncpy(requestedPath, currentCommand->arguments, 32);
			userNameSanatize(requestedPath, 32);
			requestedPath[31] = '\0';
			if(movePlayerToArea(currentCommand->caller, requestedPath) == 0)
			{
				// Call the look command after moving. It's fine to unlock, because the loop won't
				// continue until the command is queued:
				queue->lock = false;
				queueCommand(queue, "look", "", 5, 0, currentCommand->caller);
				queue->lock = true;
			}
		}
	}

	// Look command: Returns the description of the current area and paths:
	if(strncmp(currentCommand->command, "look", 4) == 0)
	{
		char formattedString[64];
		userMessage * lookMessage = calloc(1, sizeof(userMessage));
		lookMessage->senderName[0] = '\0';
		strncat(lookMessage->messageContent, currentCommand->caller->currentArea->areaName, 33);
		strncat(lookMessage->messageContent, "\n", 2);
		strncat(lookMessage->messageContent, currentCommand->caller->currentArea->areaDescription, MAX - 35);
		queueTargetedOutputMessage(parameters->outputQueue, lookMessage, &currentCommand->caller, 1);
		bzero(lookMessage, sizeof(userMessage));
		if(currentCommand->caller->currentArea->areaExits[0] != NULL)
		{
			strncat(lookMessage->messageContent, "You can go:", 13);
			for(int index = 0; index < 16; index++)
			{
				if(currentCommand->caller->currentArea->areaExits[index] != NULL)
				{
					snprintf(formattedString, 64, "\n\t%d. %s", index + 1, currentCommand->caller->currentArea->areaExits[index]->pathName);
					strncat(lookMessage->messageContent, formattedString, 64);
				}
			}	   
			queueTargetedOutputMessage(parameters->outputQueue, lookMessage, &currentCommand->caller, 1);
		}
		free(lookMessage);
	}
	// Join command: Allows the player to join the game given a name:
	// TODO: Implement login/character creation. Will be a while:
	if(strncmp(currentCommand->command, "join", 4) == 0)
	{
		if(currentCommand->caller->currentArea == getAreaFromList(parameters->areaList, 0))
		{
			bool validName = true;
			for(int index = 0; index < *parameters->playerCount; index++)
			{
				if(currentCommand->arguments[0] == '\0')
				{
					validName = false;
				}
				if(strncmp(currentCommand->arguments, parameters->connectedPlayers[index].playerName, 16) == 0)
				{
					validName = false;
				}
			}
			if(validName)
			{
				strncpy(currentCommand->caller->playerName, currentCommand->arguments, 16);
				currentCommand->caller->currentArea = getAreaFromList(parameters->areaList, 1);
				// Call the look command after joining. It's fine to unlock, because the loop won't
				// continue until the command is queued:
				queue->lock = false;
				queueCommand(queue, "look", "", 5, 0, currentCommand->caller);
				queue->lock = true;
			}
		}
	}
	// Talk command: Allows the player to begin a chat session with another player:
	if(strncmp(currentCommand->command, "talk", 4) == 0)
	{
		// TODO: Implement.
	}
	if(strncmp(currentCommand->command, "skillissue", 10) == 0)
	{
		userMessage * statMessage = calloc(1, sizeof(userMessage));
		statMessage->senderName[0] = '\0';
		strcpy(statMessage->messageContent, "Have you tried getting good?");
		queueTargetedOutputMessage(parameters->outputQueue, statMessage, &currentCommand->caller, 1);
		free(statMessage);
	}
	// Stat command: Displays the current character's sheet.
	if(strncmp(currentCommand->command, "stat", 4) == 0)
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
		if(currentCommand->caller->stats->specPoints > 0 || currentCommand->caller->stats->skillPoints > 0)
		{
			snprintf(formattedString, 120, "Current Experience: %ld | Spec Points Available: %d | Skill Points Available: %d",
					 currentCommand->caller->stats->experience, currentCommand->caller->stats->specPoints, currentCommand->caller->stats->skillPoints);
		}
		else
		{
			snprintf(formattedString, 120, "Current Experience: %ld", currentCommand->caller->stats->experience);
		}
		strncat(statMessage->messageContent, formattedString, 120);
		
		queueTargetedOutputMessage(parameters->outputQueue, statMessage, &currentCommand->caller, 1);
		bzero(statMessage->messageContent, sizeof(char) * MAX);
		if(currentCommand->caller->skills->head != NULL)
		{			
			skillNode * currentSkill = currentCommand->caller->skills->head;
			int charCount = 0;
			bool addNewline = false;
			while(currentSkill != NULL)
			{
				snprintf(formattedString, 120, "| %2d | %31s ",
						 currentSkill->skill->skillPoints, currentSkill->skill->skillName);
				charCount += 43;
				strncat(statMessage->messageContent, formattedString, 120);
				if((charCount + 43) >= MAX)
				{
					strncat(statMessage->messageContent, "\n", 2);
					queueTargetedOutputMessage(parameters->outputQueue, statMessage, &currentCommand->caller, 1);
					bzero(statMessage, sizeof(userMessage));
					charCount = 0;
					break;
				}
				else if(addNewline)
				{
					strncat(statMessage->messageContent, "|\n", 3);
					charCount++;
					addNewline = false;
				}
				else
				{
					addNewline = true;
				}
				currentSkill = currentSkill->next;
			
			}
			queueTargetedOutputMessage(parameters->outputQueue, statMessage, &currentCommand->caller, 1);
		}
		free(statMessage);
		free(formattedString);
	}
	
	// Spec command: Assign spec points to stats:
	if(strncmp(currentCommand->command, "spec", 4) == 0)
	{
		userMessage * specMessage = calloc(1, sizeof(userMessage));
		specMessage->senderName[0] = '\0';
		char * formattedString = calloc(121, sizeof(char));
		if(currentCommand->caller->stats->specPoints > 0)
		{
			int selectedAmount = 0;
			strtok(currentCommand->arguments, " ");
			selectedAmount = atoi(&currentCommand->arguments[strlen(currentCommand->arguments) + 1]);
			coreStat selectedStat = getCoreStatFromString(currentCommand->arguments, 16);
			if(selectedAmount > 0 && (currentCommand->caller->stats->specPoints - selectedAmount) >= 0)
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
		
		// Send the message:
		queueTargetedOutputMessage(parameters->outputQueue, specMessage, &currentCommand->caller, 1);

		// Show the new stat sheet:
		queue->lock = false;
		queueCommand(queue, "stat", "", 5, 0, currentCommand->caller);
		queue->lock = true;
		
		// Free the finished message:
		free(specMessage);
		free(formattedString);
	}
	if(strncmp(currentCommand->command, "skill", 5) == 0)
	{
		userMessage * skillMessage = calloc(1, sizeof(userMessage));
		skillMessage->senderName[0] = '\0';
		if((currentCommand->caller->stats->skillPoints - 1) >= 0)
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
		queueTargetedOutputMessage(parameters->outputQueue, skillMessage, &currentCommand->caller, 1);
		free(skillMessage);
	}
	if(strncmp(currentCommand->command, "listskills", 10) == 0)
	{
		skillNode * currentSkill = parameters->globalSkillList->head;
		userMessage * listMessage = calloc(1, sizeof(userMessage));
		char * formattedString = calloc(121, sizeof(char));
		int charCount = 0;
		bool addNewline = false;
		while(currentSkill != NULL)
		{
			snprintf(formattedString, 120, "| %-31s ", currentSkill->skill->skillName);
			charCount += 43;
			strncat(listMessage->messageContent, formattedString, 120);
			if((charCount + 46) >= MAX)
			{
				queueTargetedOutputMessage(parameters->outputQueue, listMessage, &currentCommand->caller, 1);
				bzero(listMessage, sizeof(userMessage));
				charCount = 0;
				addNewline = false;
			}
			else if(addNewline)
			{
				strncat(listMessage->messageContent, "|\n", 3);
				charCount++;
				addNewline = false;
			}
			else
			{
				addNewline = true;
			}
			currentSkill = currentSkill->next;
		}
		queueTargetedOutputMessage(parameters->outputQueue, listMessage, &currentCommand->caller, 1);
		free(listMessage);
		free(formattedString);
	}
	// Remove the current command and unlock the queue:
	currentCommand = NULL;
	queue->lock = false;	
	dequeueCommand(queue);
	return 0;
}

// Run a stat check:
outcome statCheck(playerInfo * player, int chance, coreStat statToCheck)
{
	// Calculate the chance:
	if(chance > 100 || chance < 0)
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
	if(attempt >= chance)
	{
		if(attempt >= 98)
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
		if(attempt <= 2)
		{
			return CRITICAL_FAILURE;
		}
		else
		{
			return FAILURE;
		}
	}
}

// Run a skill check:
outcome skillCheck(playerInfo * player, int chance, char * skillName, size_t skillNameLength, skillList * globalSkillList)
{
	// Calculate the chance:
	if(chance > 100 || chance < 0)
	{
		return ERROR;
	}
	chance = 100 - chance;

	// Check if the player has the given skill:
	bool playerHasSkill = false;
	skillNode * currentPlayerNode = player->skills->head;
	while(currentPlayerNode != NULL)
	{
		if(strncmp(skillName, currentPlayerNode->skill->skillName, skillNameLength) == 0)
		{
			playerHasSkill = true;
			break;
		}
		currentPlayerNode = currentPlayerNode->next;
	}

	// If the player doesn't have the skill, check if it's in the game and is trained:
	bool trainedSkill = false;
	if(!playerHasSkill)
	{
		skillNode * currentNode = globalSkillList->head;
		while(strncmp(skillName, currentNode->skill->skillName, 32) != 0)
		{
			if(currentNode->next == NULL)
			{
				fprintf(stderr, "Skill doesn't exist in skill list.\n");
				return ERROR;
			}
			currentNode = currentNode->next;
		}
		if(currentNode->skill->trainedSkill == true)
		{
			trainedSkill = true;
		}
	}
		
	// Calculate the modifier:
	int modifier = 0;
	if(trainedSkill)
	{
		modifier = -100;
	}
	else
	{
		modifier = currentPlayerNode->skill->skillPoints * 4;
	}
	
	// Attempt the check:
	int attempt = (random() % 100) + modifier;
	if(attempt >= chance)
	{
		if(attempt >= 98)
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
		if(attempt <= 2)
		{
			return CRITICAL_FAILURE;
		}
		else
		{
			return FAILURE;
		}
	}
}

// Move a player to a different area given a path in the area:
int movePlayerToArea(playerInfo * player, char * requestedPath)
{
	// Check if a number was given first:
	int selected = atoi(requestedPath);
	if(selected != 0)
	{
		if(player->currentArea->areaExits[selected - 1] != NULL &&
		   player->currentArea->areaExits[selected - 1]->areaToJoin != NULL)
		{
			player->currentArea = player->currentArea->areaExits[selected - 1]->areaToJoin;
			return 0;
		}
		else
		{
			return 1;
		}
	}

	// Otherwise search for the description:
	for (int index = 0; index < 16; index++)
	{
		if(player->currentArea->areaExits[index] != NULL)
		{
			if(strncmp(player->currentArea->areaExits[index]->pathName, requestedPath, 32) == 0)
			{
				printf("%s: %s\n", player->playerName, player->currentArea->areaExits[index]->pathName);
				player->currentArea = player->currentArea->areaExits[index]->areaToJoin;
				return 0;
			}
		}   
	}
	return 1;
}
