// =========================================
// | SilverMUD Server - player-data.c      |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "player-data.h"

// Internal Functions:
// ===================
static struct PlayerListNode * findMiddle(struct PlayerListNode * start, struct PlayerListNode * end)
{
	while (start != end)
	{
		start = start->next;
		if(start == end)
		{
			return start;
		}
		end = end->previous;
	}

	return start;
}
// ====================

// Allocates and sets up a new player according to the world's starter character sheet:
struct Player * createNewPlayer(struct ClientConnection * connection)
{
	struct Player * newPlayer = calloc(1, sizeof(struct Player));
	newPlayer->connection = connection;
	
	return newPlayer;
}

// Deallocates a player:
void deallocatePlayer(struct Player ** player)
{
	free(*player);
	*player = NULL;
}

struct PlayerList * createPlayerList()
{
	struct PlayerList * newPlayerList = calloc(1, sizeof(struct PlayerList));
	newPlayerList->count = 0;
	newPlayerList->head = NULL;
	newPlayerList->tail = NULL;

	return newPlayerList;
}

void deallocatePlayerList(struct PlayerList ** playerList)
{
	struct PlayerListNode * node = (*playerList)->head, * nextNode;

	// Deallocate all nodes in the list:
	while (node != NULL)
	{
		nextNode = node->next;
		free(node);
		node = nextNode;
	}

	// Deallocate the list itself:
	free(*playerList);

	// Set the pointer to null:
	playerList = NULL;		
}

int addToPlayerList(struct Player * player, struct PlayerList * playerList)
{
	// Check that the player isn't already in the list:
	if (isInPlayerList(player, playerList))
	{
		return playerList->count;
	}
	else
	{
		// Create a node to add to the list:
		struct PlayerListNode * newNode = calloc(1, sizeof(struct PlayerListNode));
		newNode->player = player;
		newNode->next = NULL;
		newNode->previous = NULL;
		
		// Find the position that the new node is to go into:

		// If the list is empty:
		if (playerList->count == 0)
		{
			playerList->head = newNode;
			playerList->tail = newNode;
			playerList->count = 1;
			return playerList->count;
		}

		struct PlayerListNode * currentNode = playerList->head;
		while (strncmp(player->name, currentNode->player->name, 64) < 0)
		{
			// If we reach the end of the list:
			if (currentNode->next == NULL)
			{
				currentNode->next = newNode;
				newNode->previous = currentNode;
				playerList->tail = newNode;
				playerList->count++;

				return playerList->count;
			}

			else
			{
				currentNode = currentNode->next;
			}
		}

		// Set the appropriate pointers in the new node:
		newNode->previous = currentNode->previous;
		currentNode->previous = newNode;
		newNode->next = currentNode;

		// Set the proper pointers if we're at the ends of the list:
		if (newNode->previous == NULL)
		{
			playerList->head = newNode;
		}
		if (newNode->next == NULL)
		{
			playerList->tail = newNode;
		}

		playerList->count++;

		return playerList->count;
	}
}

int removeFromPlayerList(struct Player * player, struct PlayerList * playerList)
{
	struct PlayerListNode * currentNode = playerList->head;
	while (currentNode != NULL)
	{
		if (currentNode->player == player)
		{
			// Adjust the proper pointers:
			if (currentNode->previous)
			{
				currentNode->previous->next = currentNode->next;
			}
			if (currentNode->next)
			{
				currentNode->next->previous = currentNode->previous;					
			}

			// Handle the special case of the head and tail of the list:
			if (playerList->head == currentNode)
			{
				playerList->head == playerList->head->next;
			}
			if (playerList->tail == currentNode)
			{
				playerList->tail == playerList->tail->previous;
			}

			// Handle the special case of an empty list:
			if (playerList->count - 1 == 0)
			{
				playerList->head = NULL;
				playerList->tail = NULL;
			}
			
			// Delete the node:
			free(currentNode);

			return --(playerList->count);
		}
		currentNode = currentNode->next;
	}
}


// Returns the Player with the given name from a PlayerList, or NULL otherwise:
struct Player * getFromPlayerList(char * playerName, struct PlayerList * playerList)
{
	struct PlayerListNode * start = playerList->head, * end = playerList->tail, * middle = findMiddle(start, end);
	int returnValue = 0;
	
	while (start != end)
	{
		returnValue = strncmp(middle->player->name, playerName, 64);

		if (returnValue < 0)
		{
			start = middle->next;
			middle = findMiddle(start, end);
		}
		else if (returnValue > 0)
		{
			end = middle->next;
			middle = findMiddle(start, end);
		}
		else if (returnValue == 0)
		{
			return middle->player;
		}
	}
	if (strncmp(start->player->name, playerName, 64) == 0)
	{
		return start->player;
	}
	else
	{
		return NULL;
	}
}

// Returns true if the given Player is in the given PlayerList:
bool isInPlayerList(struct Player * player, struct PlayerList * playerList)
{
	struct PlayerListNode * currentNode = playerList->head;
	while (currentNode != NULL)
	{
		if (currentNode->player == player)
		{
			return true;
		}
		currentNode = currentNode->next;
	}
	return false; 
}
	
// ===================================================
// | End of player-data.c, copyright notice follows. |
// ===================================================

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
