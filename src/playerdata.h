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
typedef struct playerArea playerArea;
typedef struct playerPath playerPath;
typedef struct listNode listNode;
typedef struct list list;

// The basic information that needs to be stored for a player or creature's stats:
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

// Information about a skill, including skill levels and modifiers for the player:
typedef struct playerSkill
{
	char skillName[32];
	int skillPoints;
	int skillModifier;
	bool trainedSkill;
} playerSkill;

// Information about a single player's character:
typedef struct playerInfo
{
	char playerName[32];
	playerArea * currentArea;
	statBlock * stats;
	list * skills; 
} playerInfo;

// An enum of the main stats of the game:
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

// Deallocate a player's information including the skill lists and stats:
int deallocatePlayer(playerInfo * playerToDeallocate);

#endif
