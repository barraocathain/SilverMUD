// =========================================
// | SilverMUD Server - player-data.c      |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdlib.h>
#include "player-data.h"

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
