// Implementation of lists library for SilverMUD.
// Barry Kane, 2021
#include "lists.h"
#include "playerdata.h"

areaNode * createAreaList(playerArea * initialArea)
{
	areaNode * newAreaList = malloc(sizeof(areaNode));
	newAreaList->data = initialArea;
	newAreaList->next = NULL;
	newAreaList->prev = NULL;
	return newAreaList;
}


pathNode * createPathList(playerPath * initialPath)
{
	pathNode * newPathList = malloc(sizeof(pathNode));
	newPathList->data = initialPath;
	newPathList->next = NULL;
	newPathList->prev = NULL;
	return newPathList;
}

int addAreaNodeToList(areaNode * toList, playerArea * areaToAdd)
{
	areaNode * current;
	current = toList;
	while(current->next != NULL)
	{
		current = current->next;
	}
	current->next = malloc(sizeof(areaNode));
	current->next->prev = current;
	current->next->data = areaToAdd;
	current->next->next = NULL;
}

int deleteAreaNodeFromList(areaNode * fromList, playerArea * areaToDelete)
{
	areaNode * current = fromList;
	while(current->data != areaToDelete && current->next != NULL)
	{
		current = current->next;
	}
	if(current->next == NULL && current->data != areaToDelete)
	{
		return 0;
	}
	current->prev->next = current->next;
	if(current->next != NULL)
	{
		current->next->prev = current->prev;
	}
	free(current);
}

int addPathNodeToList(pathNode * toList, playerPath * pathToAdd)
{
	pathNode * current;
	current = toList;
	while(current->next != NULL)
	{
		current = current->next;
	}
	current->next = malloc(sizeof(pathNode));
	current->next->prev = current;
	current->next->data = pathToAdd;
	current->next->next = NULL;
}

int deletePathNodeFromList(pathNode * fromList, playerPath * pathToDelete)
{
	pathNode * current = fromList;
	while(current->data != pathToDelete || current->next != NULL)
	{
		current = current->next;
	}
	if(current->next == NULL && current->data != pathToDelete)
	{
		return 0;
	}
	current->prev->next = current->next;
	if(current->next != NULL)
	{
		current->next->prev = current->prev;
	}
	free(current);
}

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

playerArea * getAreaFromList(areaNode * fromList, int listIndex)
{
	areaNode * current = getAreaNode(fromList, listIndex);
	return current->data;
}

