// playerdata.h: Header file containing data structures for player data and function
// prototypes for interacting with said data.
#ifndef PLAYERDATA_H
#define PLAYERDATA_H
#include <stdlib.h>
#include <stdbool.h>
#include "constants.h"

typedef struct playerPath playerPath;
typedef struct playerArea playerArea;

struct playerPath
{
	char pathName[32];
	playerArea * areaToJoin;
};

struct playerArea
{
	char areaName[32];
	char areaDescription[MAX - 35];
	playerPath * areaExits[16];
};

typedef struct statBlock
{
	// Levelling:
	int level;
	long experience;

	// Health:
	int currentHealth;
	int maxHealth;

	// Core Stats:
	int wits;
	int intellect;
	int strength;
	int endurance;
	int dexerity;
	
	// Character Building:
	int specPoints;
	int skillPoints; 
} statBlock;

typedef struct playerSkill
{
	char skillName[32];
	int skillPoints;
	int skillModifier;
	bool trainedSkill;
} playerSkill;

typedef struct skillNode skillNode;
struct skillNode
{
	playerSkill * skill;
	skillNode * next;
};

typedef struct skillList
{
	skillNode * head;
	int skillCount;
} skillList;

typedef struct playerInfo
{
	char playerName[32];
	playerArea * currentArea;
	statBlock * stats;
	skillList * skills; 
} playerInfo;

typedef enum coreStat
{
	WITS,
	INTELLECT,
	STRENGTH,
	ENDURANCE,
	DEXERITY,
	INVALID
} coreStat;

// Move a player to a different area given a path in the area:
int movePlayerToArea(playerInfo * player, char * requestedPath);

// Create an area given a name and description:
playerArea * createArea(char * nameString, char * descriptionString);

// Create a path between two areas given two areas and two strings:
int createPath(playerArea * fromArea, playerArea * toArea, char * fromDescription, char * toDescription);

// Create a new skill and add it to the global skill list:
int createSkill(skillList * globalSkillList, char * skillName, int skillNameLength, bool trainedSkill);

// Add a skill node to a skill list:
int addSkillNode(skillList * skillList, playerSkill * skill);

// Remove a skill node from a skill list:
int removeSkillNode(skillList * skillList, playerSkill * skill);
int removeSkillByID(skillList * skillList, playerSkill * skill);

// Take a skill and add it to the player's skill list:
int takeSkill(skillList * globalSkillList, char * skillName, int skillNameLength, playerInfo * targetPlayer); 
int takeSkillbyID(skillList * globalSkillList, int skillID, playerInfo * targetPlayer);

// Take a string containing a core stat name and return the core stat:
coreStat getCoreStatFromString(char * string, int stringLength);

// Deallocate a player:
int deallocatePlayer(playerInfo * playerToDeallocate);
#endif
