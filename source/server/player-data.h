// =========================================
// | SilverMUD Server - player-data.h      |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H
#define PLAYER_NAME_LENGTH 64
#include <stdbool.h>
#include "connections.h"

// =================================================================
// Players - A structure for representing a single player character:
// =================================================================
struct Player
{
	struct ClientConnection * connection;
	char * name;
};

// Functions:
// ==========

// Allocates and sets up a new player according to the world's starter character sheet:
struct Player * createNewPlayer(struct ClientConnection * connection);

// Deallocates a player:
void deallocatePlayer(struct Player ** player);

// ========================================================================================
// Player Lists - A structure for managing a collection of players in a doubly linked list:
// ========================================================================================
struct PlayerListNode
{
  	struct Player * player;
	struct PlayerListNode * next, * previous;
};

struct PlayerList
{
	size_t count;
	struct PlayerListNode * head, * tail;
};

// Functions:
// ==========

struct PlayerList * createPlayerList();
void deallocatePlayerList(struct PlayerList ** playerList);

// Adds a Player into a PlayerList, in a sorted position by character name.
// Returns the count of players in the list:
int addToPlayerList(struct Player * player, struct PlayerList * playerList);

// Remove a Player from a PlayerList. Returns the count of players in the list:
int removeFromPlayerList(struct Player * player, struct PlayerList * playerList);

// Returns the Player with the given name from a PlayerList, or NULL otherwise:
struct Player * getFromPlayerList(char * playerName, struct PlayerList * playerList);

// Returns true if the given Player is in the given PlayerList:
bool isInPlayerList(struct Player * player, struct PlayerList * playerList);

#endif
// ===================================================
// | End of player-data.h, copyright notice follows. |
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
