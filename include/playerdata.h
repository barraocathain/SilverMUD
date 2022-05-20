// playerdata.h: Header file containing data structures for player data and function
// prototypes for interacting with said data.
#ifndef PLAYERDATA_H
#define PLAYERDATA_H
#include <stdlib.h>

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

// Create a path between two areas given two areas and two strings:
int createPath(playerArea * fromArea, playerArea * toArea, char * fromDescription, char * toDescription);

#endif
