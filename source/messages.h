// =========================================
// | SilverMUD - messages.h                |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef MESSAGES_H
#define MESSAGES_H
#include <libguile.h> 
#define MESSAGE_NAME_LENGTH 128
#define MESSAGE_CONTENT_LENGTH 1024

enum MessageTypes
{
	SYSTEM,
	CLIENT_SETTING,
	COMMAND_OUTPUT,
	LOCAL_CHAT,
	PLAYER_CHAT,
	PARTY_CHAT
};

struct ClientToServerMessage
{
	char content[MESSAGE_CONTENT_LENGTH];
};

struct ServerToClientMessage
{
	uint8_t type;
	char name[MESSAGE_NAME_LENGTH];
	char content[MESSAGE_CONTENT_LENGTH];
};

// Allocate and initialize a client to server message using the passed values:
struct ClientToServerMessage * createClientToServerMessage(char * content);
SCM scheme_createClientToServerMessage(SCM content);

 // Allocate and initialize a server to client message using the passed values:
struct ServerToClientMessage * createServerToClientMessage(uint8_t type, char * name, char * content);
SCM scheme_createServerToClientMessage(SCM type, SCM name, SCM content);

#endif
// ================================================
// | End of messages.h, copyright notice follows. |
// ================================================

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
