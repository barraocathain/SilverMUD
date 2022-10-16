// areadata.h: Contains data structures and functions for playerAreas and playerPaths in SilverMUD:
// Barra Ó Catháin, 2022.
#ifndef AREADATA_H
#define AREADATA_H
#include "playerdata.h"

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

areaNode * createAreaList(playerArea * initialArea);

pathNode * createPathList(playerPath * initialPath);

int addAreaNodeToList(areaNode * toList, playerArea * areaToAdd);

int deleteAreaNodeFromList(areaNode * fromList, playerArea * areaToDelete);

int addPathNodeToList(pathNode * toList, playerPath * pathToAdd);

int deletePathNodeFromList(pathNode * fromList, playerPath * pathToDelete);

areaNode * getAreaNode(areaNode * fromList, int listIndex);

pathNode * getPathNode(pathNode * fromList, int listIndex);

playerArea * getAreaFromList(areaNode * fromList, int listIndex);

// TO BE IMPLEMENTED:
/* int saveAreaList(areaNode * listToSave); */

/* int savePathList(pathNode * listToSave); */

/* int loadAreaList(areaNode * listToLoad); */

/* int loadPathList(pathNode * listToLoad); */

#endif
