// =========================================
// | SilverMUD Server - connections.h      |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef CONNECTIONS_H
#define CONNECTIONS_H
#include <stddef.h>
#include <gnutls/gnutls.h>

struct ClientConnection
{	
	gnutls_session_t * tlsSession;
	struct Player * player;
	int fileDescriptor;	
};

struct ClientConnectionNode
{
	struct ClientConnection * connection;
	struct ClientConnectionNode * next;
	struct ClientConnectionNode * previous;
};

struct ClientConnectionList
{
	size_t clientCount;
	struct ClientConnectionNode * head;
	struct ClientConnectionNode * tail;
};


//struct ClientConnection * findConnectionByPlayer(struct ClientConnectionList * list);
struct ClientConnection * findConnectionByFileDescriptor(struct ClientConnectionList * list, int fileDescriptor);
struct ClientConnection * findConnectionByTlsSession(struct ClientConnectionList * list, gnutls_session_t * tlsSession);

int removeConnectionByFileDescriptor(struct ClientConnectionList * list, int fileDescriptor);
struct ClientConnection * addNewConnection(struct ClientConnectionList * list, int fileDescriptor, gnutls_session_t * tlsSession);

#endif
// ===================================================
// | End of connections.h, copyright notice follows. |
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
