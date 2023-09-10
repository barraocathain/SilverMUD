// =========================================
// | SilverMUD Server - player-data.h      |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H
#include "connections.h"

struct Player
{
	struct ClientConnection * connection;
	char name[64];
};

// Allocates and sets up a new player according to the world's starter character sheet:
struct Player * createNewPlayer(struct ClientConnection * connection);

// Deallocates a player:
void deallocatePlayer(struct Player ** player);

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
