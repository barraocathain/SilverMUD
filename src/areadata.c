// areadata.c: Implements functions for playerAreas and playerPaths in SilverMUD:
// Barra Ó Catháin, 2022.
#include <string.h>
#include "areadata.h"
#include "playerdata.h"

// ====================
// -=[ Area/Paths: ]=-:
// ====================

// Create an area given a name and description:
playerArea * createArea(char * nameString, char * descriptionString)
{
	// Allocate and zero memory for the new area:
	playerArea * createdArea = calloc(1, sizeof(playerArea));

	// Copy the strings into the newly created area:
	strncpy(createdArea->areaName, nameString, 32 - 1);
	strncpy(createdArea->areaDescription, descriptionString, MAX - 36);

	// Properly null-terminate the strings:
	createdArea->areaName[31] = '\0';
	createdArea->areaDescription[MAX - 36] = '\0';

	// Ensure that all the paths are set to NULL:
	for(int index = 0; index < 16; index++)
	{
		createdArea->areaExits[index] = NULL;
	}

	// Return the pointer:
	return createdArea;
}

// Create a path between two areas given two areas and two strings:
int createPath(playerArea * fromArea, playerArea * toArea, char * fromDescription, char * toDescription)
{
	int fromAreaSlot, toAreaSlot;
	for(fromAreaSlot = 0; fromAreaSlot < 16; fromAreaSlot++)
	{
		if(fromArea->areaExits[fromAreaSlot] == NULL)
		{
			break;
		}
		if((fromArea->areaExits[fromAreaSlot] != NULL) && (fromAreaSlot == 15))
		{
			return 1;
		}
	}
	for(toAreaSlot = 0; toAreaSlot < 32; toAreaSlot++)
	{
		if(toArea->areaExits[toAreaSlot] == 0)
		{
			break;
		}
		if((toArea->areaExits[toAreaSlot] != 0) && (toAreaSlot == 31))
		{
			return 2;
		}
	}
	playerPath * fromPath = malloc(sizeof(playerPath));
	playerPath * toPath = malloc(sizeof(playerPath));
	fromArea->areaExits[fromAreaSlot] = fromPath;
	toArea->areaExits[toAreaSlot] = toPath;
	strncpy(fromPath->pathName, fromDescription, 32 - 1);
	fromPath->pathName[31] = '\0';	
	strncpy(toPath->pathName, toDescription, 32 - 1);
	toPath->pathName[31] = '\0';	
	fromArea->areaExits[fromAreaSlot]->areaToJoin = toArea;
	toArea->areaExits[toAreaSlot]->areaToJoin = fromArea;
	return 0;
}

// =========================
// -=[ Area/Path Lists: ]=-:
// =========================

// Create and initialize an areaList:
areaNode * createAreaList(playerArea * initialArea)
{
	areaNode * newAreaList = malloc(sizeof(areaNode));
	newAreaList->data = initialArea;
	newAreaList->next = NULL;
	newAreaList->prev = NULL;
	return newAreaList;
}

// Create and initialize an pathList:
pathNode * createPathList(playerPath * initialPath)
{
	pathNode * newPathList = malloc(sizeof(pathNode));
	newPathList->data = initialPath;
	newPathList->next = NULL;
	newPathList->prev = NULL;
	return newPathList;
}

// Adds an areaNode to the end of a list, returning it's position:
int addAreaNodeToList(areaNode * toList, playerArea * areaToAdd)
{
	areaNode * current;
	int index = 0;
	current = toList;
	while(current->next != NULL)
	{
		current = current->next;
		index++;
	}
	current->next = malloc(sizeof(areaNode));
	current->next->prev = current;
	current->next->data = areaToAdd;
   	current->next->next = NULL;
	return 0;
}

// Removes an areaNode from the list, returning 0 on success and -1 on failure:
int deleteAreaNodeFromList(areaNode * fromList, playerArea * areaToDelete)
{
	areaNode * current = fromList;
	while(current->data != areaToDelete && current->next != NULL)
	{
		current = current->next;
	}
	if(current->next == NULL && current->data != areaToDelete)
	{
		return -1;
	}
	current->prev->next = current->next;
	if(current->next != NULL)
	{
		current->next->prev = current->prev;
	}
	free(current);
	return 0;
}

// Adds an pathNode to the end of a list, returning it's position:
int addPathNodeToList(pathNode * toList, playerPath * pathToAdd)
{
	pathNode * current;
	int index = 0;
	current = toList;
	while(current->next != NULL)
	{
 		current = current->next;
		index++;
	}
	current->next = malloc(sizeof(pathNode));
	current->next->prev = current;
	current->next->data = pathToAdd;
	current->next->next = NULL;
	return index;
}

// Removes an pathNode from the list, returning 0 on success and -1 on failure:
int deletePathNodeFromList(pathNode * fromList, playerPath * pathToDelete)
{
	pathNode * current = fromList;
	while(current->data != pathToDelete || current->next != NULL)
	{
		current = current->next;
	}
	if(current->next == NULL && current->data != pathToDelete)
	{
		return -1;
	}
	current->prev->next = current->next;
	if(current->next != NULL)
	{
		current->next->prev = current->prev;
	}
	free(current);
	return 0;
}

// Return the areaNode at the given index from the list:
areaNode * getAreaNode(areaNode * fromList, int listIndex)
{
	areaNode * current = fromList;
	for(int index = 0; index < listIndex; index++)
	{
		if(current->next != NULL)
		{
			current = current->next;
		}
		else
		{
			return NULL;
		}
	}
	return current;
}

// Return the pathNode at the given index from the list:
pathNode * getPathNode(pathNode * fromList, int listIndex)
{
	pathNode * current = fromList;
	for(int index = 0; index < listIndex; index++)
	{
		if(current->next != NULL)
		{
			current = current->next;
		}
		else
		{
			return NULL;
		}
	}
	return current;
}

// Return the playerArea of the areaNode at the given index from the list:
playerArea * getAreaFromList(areaNode * fromList, int listIndex)
{
	areaNode * current = getAreaNode(fromList, listIndex);
	return current->data;
}
