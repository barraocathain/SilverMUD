// playerdata.h: Header file containing data structures for player data and function
// definitions for interacting with said data.
// Barry Kane, 2021.
#ifndef PLAYERDATA_H
#define PLAYERDATA_H
#include <stdlib.h>

typedef struct userMessage
{
	char senderName[32];
	char messageContent[2048];

} userMessage;

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
	char areaDescription[256];
	playerPath * areaExits[16];
};

typedef struct playerInfo
{
	char playerName[32];
	playerArea * currentArea;
} playerInfo;

// Move a player to a different area given a path in the area:
int movePlayerToArea(playerInfo * player, char * requestedPath);

// Create an area given a name and description:
playerArea * createArea(char * nameString, char * descriptionString);

// Create a path between two areas given two areas and a string:
int createPath(playerArea * fromArea, playerArea * toArea, char * pathFromString, char * pathToString);

#endif
