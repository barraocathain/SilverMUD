// areadata.h: Contains data structures and functions for playerAreas and playerPaths in SilverMUD:
// Barra Ó Catháin, 2022.
#ifndef AREADATA_H
#define AREADATA_H
#include "constants.h"
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
	char areaName[32];
	char areaDescription[MAX - 35];
	playerPath * areaExits[16];
};

// Create an area given a name and description:
playerArea * createArea(char * nameString, char * descriptionString);

// Create a path between two areas given two areas and two strings:
int createPath(playerArea * fromArea, playerArea * toArea, char * fromDescription, char * toDescription);

// =========================
// -=[ Area/Path Lists: ]=-:
// =========================

typedef struct areaNode areaNode;
typedef struct pathNode pathNode;

struct pathNode
{
	playerPath * data;
	pathNode * next;
	pathNode * prev;
};

struct areaNode
{
	playerArea * data;
	areaNode * next;
	areaNode * prev;
};

// Create and initialize an areaList:
areaNode * createAreaList(playerArea * initialArea);

// Create and initialize an pathList:
pathNode * createPathList(playerPath * initialPath);

// Adds an areaNode to the end of a list, returning it's position:
int addAreaNodeToList(areaNode * toList, playerArea * areaToAdd);

// Removes an areaNode from the list, returning 0 on success and -1 on failure:
int deleteAreaNodeFromList(areaNode * fromList, playerArea * areaToDelete);

// Adds an pathNode to the end of a list, returning it's position:
int addPathNodeToList(pathNode * toList, playerPath * pathToAdd);

// Removes an pathNode from the list, returning 0 on success and -1 on failure:
int deletePathNodeFromList(pathNode * fromList, playerPath * pathToDelete);

// Return the areaNode at the given index from the list:
areaNode * getAreaNode(areaNode * fromList, int listIndex);

// Return the pathNode at the given index from the list:
pathNode * getPathNode(pathNode * fromList, int listIndex);

// Return the playerArea of the areaNode at the given index from the list:
playerArea * getAreaFromList(areaNode * fromList, int listIndex);

// TO BE IMPLEMENTED:
/* int saveAreaList(areaNode * listToSave); */

/* int savePathList(pathNode * listToSave); */

/* int loadAreaList(areaNode * listToLoad); */

/* int loadPathList(pathNode * listToLoad); */

#endif
