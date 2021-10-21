// playerdata.c: Contains functions definitions for working with player data.
// Barry Kane, 2021
#include <string.h>
#include "playerdata.h"

// Move a player to a different area given a path in the area.
int movePlayerToArea(playerInfo * player, char * requestedPath)
{
	for (int index = 0; index < 32; index++)
	{	    
		if(strncmp(player->currentArea->areaExits[index]->pathName, requestedPath, 32) == 0)
		{
			player->currentArea = player->currentArea->areaExits[index]->areaToJoin;
			return 0;
		}
	}
	return 1;
}

