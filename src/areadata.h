// areadata.h: Contains data structures and functions for playerAreas and playerPaths in SilverMUD:
// Barra Ó Catháin, 2022.
#ifndef AREADATA_H
#define AREADATA_H
#include "constants.h"
#include "linkedlist.h"

// ====================
// -=[ Area/Paths: ]=-:
// ====================

// Let the compiler know that we're going to define these types:
typedef struct playerPath playerPath;
typedef struct playerArea playerArea;
typedef struct list list;

// A path, which contains a name, and a pointer to the area which the player will travel to:
struct playerPath
{
	char pathName[32];
	playerArea * areaToJoin;
};

// An area, containing a list of paths exiting from the area, and a name and description of the area:
struct playerArea
{
	list * pathList;
	char areaName[32];
	char areaDescription[MAX - 35];
};

// Create an area given a name and description, returning a pointer to the new area:
playerArea * createArea(char * nameString, char * descriptionString);

// Create a path between two areas given two areas and two strings, adding it to the both area's list of paths: 
int createPath(playerArea * fromArea, playerArea * toArea, char * fromDescription, char * toDescription);

// Create a one-way path between two areas given two areas and a string, adding it to the first area's list of paths: 
int createOneWayPath(playerArea * fromArea, playerArea * toArea, char * description);

// TO BE IMPLEMENTED:
/* int saveAreaList(areaNode * listToSave); */

/* int savePathList(pathNode * listToSave); */

/* int loadAreaList(areaNode * listToLoad); */

/* int loadPathList(pathNode * listToLoad); */

#endif
