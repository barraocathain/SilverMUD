// playerdata.h: Header file containing data structures for player data and function
// prototypes for interacting with said data.
#ifndef PLAYERDATA_H
#define PLAYERDATA_H
#include <stdlib.h>
#include <stdbool.h>
#include "areadata.h"
#include "constants.h"
#include "linkedlist.h"

// Let the compiler know there will be structs defined elsewhere:
typedef struct playerPath playerPath;
typedef struct playerArea playerArea;
typedef struct list list;
typedef struct listNode listNode;

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

typedef struct playerInfo
{
	char playerName[32];
	playerArea * currentArea;
	statBlock * stats;
	list * skills; 
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

// Create a new skill and add it to the global skill list:
listNode * createSkill(list * globalSkillList, char * skillName, int skillNameLength, bool trainedSkill);

// Take a skill and add it to the player's skill list:
int takeSkill(list * globalSkillList, char * skillName, int skillNameLength, playerInfo * targetPlayer); 
int takeSkillbyID(list * globalSkillList, int skillID, playerInfo * targetPlayer);

// Take a string containing a core stat name and return the core stat:
coreStat getCoreStatFromString(char * string, int stringLength);

// Deallocate a player:
int deallocatePlayer(playerInfo * playerToDeallocate);

#endif
