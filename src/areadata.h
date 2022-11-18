// areadata.h: Contains data structures and functions for playerAreas and playerPaths in SilverMUD:
// Barra Ó Catháin, 2022.
#ifndef AREADATA_H
#define AREADATA_H
#include "constants.h"
#include "linkedlist.h"

// ====================
// -=[ Area/Paths: ]=-:
// ====================

typedef struct playerPath playerPath;
typedef struct playerArea playerArea;

struct playerPath
{
	char pathName[32];
	playerArea * areaToJoin;
};

struct playerArea
{
	list * pathList;
	char areaName[32];
	char areaDescription[MAX - 35];
//	playerPath * areaExits[16];
};

// Create an area given a name and description:
playerArea * createArea(char * nameString, char * descriptionString);

// Create a path between two areas given two areas and two strings:
int createPath(playerArea * fromArea, playerArea * toArea, char * fromDescription, char * toDescription);

// Create a one-way path between two areas given two areas and a string:
int createOneWayPath(playerArea * fromArea, playerArea * toArea, char * description);

// TO BE IMPLEMENTED:
/* int saveAreaList(areaNode * listToSave); */

/* int savePathList(pathNode * listToSave); */

/* int loadAreaList(areaNode * listToLoad); */

/* int loadPathList(pathNode * listToLoad); */

#endif
