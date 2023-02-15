// playerdata.c: Contains functions definitions for working with player data.
// Barry Kane, 2021
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "constants.h"
#include "playerdata.h"

// Create a new skill and add it to the global skill list:
listNode * createSkill(list * globalSkillList, char * skillName, int skillNameLength, bool trainedSkill)
{
	if (skillNameLength >= 32)
	{
		fprintf(stderr, "Skill name is too long. Please shorten the name and try again.\n");
		return NULL;
	}
	playerSkill * newSkill = malloc(sizeof(playerSkill));
	
	strncpy(newSkill->skillName, skillName, 31);
	newSkill->skillName[31] = '\0';
	
	newSkill->skillPoints = 0;
	newSkill->skillModifier = 0;
	newSkill->trainedSkill = trainedSkill;
	
	// Add the skill to a node in the list:
	return(addToList(globalSkillList, newSkill, SKILL));
}

// Take a skill and add it to the player's skill list:
int takeSkill(list * globalSkillList, char * skillName, int skillNameLength, playerInfo * targetPlayer)
{
	// Check if the skill exists in the game:
	size_t globalIndex = 0;
	bool skillExists = false;
	while (globalIndex < globalSkillList->itemCount)
	{
		if (strncmp(skillName, getFromList(globalSkillList, globalIndex)->skill->skillName, skillNameLength) == 0)
		{
			skillExists = true;
			break;
		}
		globalIndex++;
	}

	if (!skillExists)
	{
		fprintf(stderr, "Skill doesn't exist in skill list.\n");
		return -1;
	}

	// Check if the player has the skill:
	size_t playerIndex = 0;
	bool playerHasSkill = false;
	while (playerIndex < targetPlayer->skills->itemCount)
	{
		if (strncmp(skillName, getFromList(targetPlayer->skills, playerIndex)->skill->skillName, skillNameLength) == 0)
		{
			playerHasSkill = true;
			break;
		}
		playerIndex++;
	}
	if (playerHasSkill)
	{
		getFromList(targetPlayer->skills, playerIndex)->skill->skillPoints++;
	}

	// Copy the skill into the player's skill list:
	else
	{
		playerSkill * newSkill = calloc(1, sizeof(playerSkill));
		strncpy(newSkill->skillName, getFromList(globalSkillList, globalIndex)->skill->skillName, 32);
		printf("%s ", newSkill->skillName);
		newSkill->skillPoints = 1;
		addToList(targetPlayer->skills, newSkill, SKILL);		
	}
	return 0;
}


// Take a string containing a core stat name and return the core stat:
coreStat getCoreStatFromString(char * inputString, int stringLength)
{
	// Check we've got a long enough string to fit a stat:
	if (stringLength < 4)
	{
		return INVALID;
	}
	
	// Lowercase the string:
	char * string = malloc(sizeof(char) * stringLength);
	for(int index = 0; index < stringLength; index++)
	{
		string[index] = tolower(inputString[index]);
	}
	
	// If we have a string that's at most just the stat name plus a null character, or
	// a dirtier string, we can check in a better order and ignore impossibilites:
	if (stringLength < 9)
	{
		if (stringLength <= 4)
		{
			if (strncmp(string, "wits", 4) == 0)
			{
				free(string);
				return WITS;
			}
			else
			{
				free(string);
				return INVALID;
			}
		}
		// Hopefully one of the seven letter long ones:
		else if (stringLength <= 7)
		{
			if (strncmp(string, "strength", 7) == 0)
			{
				free(string);
				return STRENGTH;
			}
			else if (strncmp(string, "dexerity", 7) == 0)
			{
				free(string);
				return DEXERITY;
			}
			if (strncmp(string, "wits", 4) == 0)
			{
				free(string);
				return WITS;
			}
			else
			{
				free(string);
				return INVALID;
			}
		}
		// Hopefully one of the 8 letter long stats:
		else
		{
			if (strncmp(string, "intellect", 8) == 0)
			{
				free(string);
				return INTELLECT;
			}
			else if (strncmp(string, "endurance", 8) == 0)
			{
				free(string);
				return ENDURANCE;
			}
			else if (strncmp(string, "strength", 7) == 0)
			{
				free(string);
				return STRENGTH;
			}
			else if (strncmp(string, "dexerity", 7) == 0)
			{
				free(string);
				return DEXERITY;
			}
			if (strncmp(string, "wits", 4) == 0)
			{
				free(string);
				return WITS;
			}
			else
			{
				free(string);
				return INVALID;
			}
		}
	}
	// Worst case, it's definitely a dirty string, compare them all:
	else
	{
			if (strncmp(string, "wits", 4) == 0)
			{
				free(string);
				return WITS;
			}
			else if (strncmp(string, "intellect", 8) == 0)
			{
				free(string);
				return INTELLECT;
			}
			else if (strncmp(string, "strength", 7) == 0)
			{
				free(string);
				return STRENGTH;
			}
			else if (strncmp(string, "endurance", 8) == 0)
			{
				free(string);
				return ENDURANCE;
			}
			else if (strncmp(string, "dexerity", 7) == 0)
			{
				free(string);
				return DEXERITY;
			}
			else
			{
				free(string);
				return INVALID;
			}
	}
}

// Deallocate a player's information including the skill lists and stats:
int deallocatePlayer(playerInfo * playerToDeallocate)
{
	// Deallocate the skill list:
	destroyList(&(playerToDeallocate->skills));

	// Deallocate the stat block:
	free(playerToDeallocate->stats);

	// Deallocate the player:
	free(playerToDeallocate);

	return 0;
}
