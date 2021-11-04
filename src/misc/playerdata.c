// playerdata.c: Contains functions definitions for working with player data.
// Barry Kane, 2021
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "playerdata.h"

// Move a player to a different area given a path in the area:
int movePlayerToArea(playerInfo * player, char * requestedPath)
{
	for (int index = 0; index < 16; index++)
	{
		if(player->currentArea->areaExits[index] != NULL)
		{
			if(strncmp(player->currentArea->areaExits[index]->pathName, requestedPath, 32) == 0)
			{
				printf("%s: %s\n", player->playerName, player->currentArea->areaExits[index]->pathName);
				player->currentArea = player->currentArea->areaExits[index]->areaToJoin;
				return 0;
			}
		}   
	}
	return 1;
}

// Create an area given a name and description:
playerArea * createArea(char * nameString, char * descriptionString)
{
	playerArea * createdArea = calloc(1, sizeof(playerArea));
	strncpy(createdArea->areaName, nameString, 32);
	strncpy(createdArea->areaDescription, descriptionString, 256);
	for(int index = 0; index < 16; index++)
	{
		createdArea->areaExits[index] = NULL;
	}
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
	playerPath * fromPath = calloc(1, sizeof(playerPath));
	playerPath * toPath = calloc(1, sizeof(playerPath));
	fromArea->areaExits[fromAreaSlot] = fromPath;
	toArea->areaExits[toAreaSlot] = toPath;
	strncpy(fromArea->areaExits[fromAreaSlot]->pathName, fromDescription, 32);
	strncpy(toArea->areaExits[toAreaSlot]->pathName, toDescription, 32);
	fromArea->areaExits[fromAreaSlot]->areaToJoin = toArea;
	toArea->areaExits[toAreaSlot]->areaToJoin = fromArea;
} 

