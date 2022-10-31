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
int createSkill(skillList * globalSkillList, char * skillName, int skillNameLength, bool trainedSkill)
{
	if(skillNameLength >= 32)
	{
		fprintf(stderr, "Skill name is too long. Please shorten the name and try again.\n");
		return -1;
	}
	playerSkill * newSkill = malloc(sizeof(playerSkill));
	
	strncpy(newSkill->skillName, skillName, 31);
	newSkill->skillName[31] = '\0';
	
	newSkill->skillPoints = 0;
	newSkill->skillModifier = 0;
	newSkill->trainedSkill = trainedSkill;
	
	// Add the skill to a node in the list:
	return(addSkillNode(globalSkillList, newSkill));
}

// Add a skill node to a skill list:
int addSkillNode(skillList * skillList, playerSkill * skill)
{
	if(skillList->head == NULL)
	{
		skillList->head = malloc(sizeof(skillNode));
		skillList->head->skill = skill;
		skillList->head->next = NULL;
		skillList->skillCount = 1;
		return 0;
	}
 	else
	{
		skillNode * currentNode = skillList->head;	
		while(currentNode->next != NULL)
		{
			currentNode = currentNode->next;
		}
		currentNode->next = malloc(sizeof(skillNode));
		currentNode->next->skill = skill;
		currentNode->next->next = NULL;
		skillList->skillCount++;
		return skillList->skillCount;		
	}
}

// Remove a skill node from a skill list:
int removeSkillNode(skillList * skillList, playerSkill * skill)
{
	// Check the validity of the pointers:
	if(skillList->head == NULL || skill == NULL)
	{
		return -1;
	}

	if(skillList->head->skill == skill)
	{
		skillNode * newHead = skillList->head->next;
		free(skillList->head->skill);
		free(skillList->head);
		skillList->head = newHead;
		return 0;
	}
	
	else
	{
		skillNode * currentNode = skillList->head;
		skillNode * previousNode = skillList->head;
		while(currentNode->skill != skill)
		{
			if(currentNode->next == NULL)
			{
				return -1;
			}
			previousNode = currentNode;
			currentNode = currentNode->next;
		}
		free(currentNode->skill);
		previousNode->next = currentNode->next;
		free(currentNode);
		return 0;
	}	
}

// Take a skill and add it to the player's skill list:
int takeSkill(skillList * globalSkillList, char * skillName, int skillNameLength, playerInfo * targetPlayer)
{
	
	skillNode * currentNode = globalSkillList->head;
	while(strncmp(skillName, currentNode->skill->skillName, 32) != 0)
	{
		if(currentNode->next == NULL)
		{
			fprintf(stderr, "Skill doesn't exist in skill list.\n");
			return -1;
		}
		currentNode = currentNode->next;
	}

	bool playerHasSkill = false;
	skillNode * currentPlayerNode = targetPlayer->skills->head;
	while(currentPlayerNode != NULL)
	{
		if(strncmp(skillName, currentPlayerNode->skill->skillName, skillNameLength) == 0)
		{
			playerHasSkill = true;
			break;
		}
		currentPlayerNode = currentPlayerNode->next;
	}
	if(playerHasSkill)
	{
		currentPlayerNode->skill->skillPoints++;
	}
	else
	{
		addSkillNode(targetPlayer->skills, currentNode->skill);
		currentPlayerNode = targetPlayer->skills->head;
		while(currentPlayerNode->next != NULL)
		{
			currentPlayerNode = currentPlayerNode->next;
		}
		currentPlayerNode->skill->skillPoints = 1;
	}
	return 0;
}


// Take a string containing a core stat name and return the core stat:
coreStat getCoreStatFromString(char * inputString, int stringLength)
{
	// Check we've got a long enough string to fit a stat:
	if(stringLength < 4)
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
	if(stringLength < 9)
	{
		if(stringLength <= 4)
		{
			if(strncmp(string, "wits", 4) == 0)
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
		else if(stringLength <= 7)
		{
			if(strncmp(string, "strength", 7) == 0)
			{
				free(string);
				return STRENGTH;
			}
			else if(strncmp(string, "dexerity", 7) == 0)
			{
				free(string);
				return DEXERITY;
			}
			if(strncmp(string, "wits", 4) == 0)
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
			if(strncmp(string, "intellect", 8) == 0)
			{
				free(string);
				return INTELLECT;
			}
			else if(strncmp(string, "endurance", 8) == 0)
			{
				free(string);
				return ENDURANCE;
			}
			else if(strncmp(string, "strength", 7) == 0)
			{
				free(string);
				return STRENGTH;
			}
			else if(strncmp(string, "dexerity", 7) == 0)
			{
				free(string);
				return DEXERITY;
			}
			if(strncmp(string, "wits", 4) == 0)
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
			if(strncmp(string, "wits", 4) == 0)
			{
				free(string);
				return WITS;
			}
			else if(strncmp(string, "intellect", 8) == 0)
			{
				free(string);
				return INTELLECT;
			}
			else if(strncmp(string, "strength", 7) == 0)
			{
				free(string);
				return STRENGTH;
			}
			else if(strncmp(string, "endurance", 8) == 0)
			{
				free(string);
				return ENDURANCE;
			}
			else if(strncmp(string, "dexerity", 7) == 0)
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

int deallocatePlayer(playerInfo * playerToDeallocate)
{
	// Deallocate the skill list:
	if(playerToDeallocate->skills->skillCount > 0)
	{
		// Allocate enough pointers:
		skillNode * nodesToDeallocate[playerToDeallocate->skills->skillCount];
		skillNode * currentSkillNode = playerToDeallocate->skills->head;

		// Get a list of all the nodes together:
		for(int index = 0; index < playerToDeallocate->skills->skillCount; index++)
		{
			nodesToDeallocate[index] = currentSkillNode;
			currentSkillNode = currentSkillNode->next;
		}

		// Deallocate all the nodes:
		for(int index = 0; index < playerToDeallocate->skills->skillCount; index++)
		{
			free(nodesToDeallocate[index]);
		}		
	}

	// Deallocate the stat block:
	free(playerToDeallocate->stats);

	// Deallocate the player:
	free(playerToDeallocate);

	return 0;
}
