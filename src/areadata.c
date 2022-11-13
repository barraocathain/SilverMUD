// areadata.c: Implements functions for playerAreas and playerPaths in SilverMUD:
// Barra Ó Catháin, 2022.
#include <string.h>
#include "areadata.h"
#include "playerdata.h"
#include "linkedlist.h"

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

	// Create a list for the paths in the area:
	createdArea->pathList = createList(PATH);
	
	// Return the pointer:
	return createdArea;
}

// Create a path between two areas given two areas and two strings:
int createPath(playerArea * fromArea, playerArea * toArea, char * fromDescription, char * toDescription)
{
	// Allocate the new paths:
	playerPath * fromPath = malloc(sizeof(playerPath));
	playerPath * toPath = malloc(sizeof(playerPath));

	// Setup the from path:
	strncpy(fromPath->pathName, fromDescription, 32 - 1);
	fromPath->pathName[31] = '\0';
	fromPath->areaToJoin = toArea;
   
	// Setup the to path:
	strncpy(toPath->pathName, toDescription, 32 - 1);
	toPath->pathName[31] = '\0';
	toPath->areaToJoin = fromArea;

	// Add to the lists:
	addToList(fromArea->pathList, fromPath, PATH);
	addToList(toArea->pathList, toPath, PATH);
	
	return 0;
}

// Create a one-way path between two areas given two areas and a string:
int createOneWayPath(playerArea * fromArea, playerArea * toArea, char * description)
{
	// Allocate the new paths:
	playerPath * path = calloc(1, sizeof(playerPath));

	// Setup the path:
	strncpy(path->pathName, description, 32 - 1);
	path->pathName[31] = '\0';
	path->areaToJoin = toArea;

	// Add to the list:
	addToList(fromArea->pathList, path, PATH);
	
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

// Return the playerArea of the areaNode at the given index from the list:
playerArea * getAreaFromList(areaNode * fromList, int listIndex)
{
	areaNode * current = getAreaNode(fromList, listIndex);
	return current->data;
}
