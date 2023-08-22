// =========================================
// | SilverMUD Server - connections.c      |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdlib.h>
#include <gnutls/gnutls.h>

#include "connections.h"

struct ClientConnectionNode * findMiddle(struct ClientConnectionNode * start, struct ClientConnectionNode * end)
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

struct ClientConnection * findConnectionByFileDescriptor(struct ClientConnectionList * list, int fileDescriptor)
{
	struct ClientConnectionNode * start = list->head, * end = list->tail, * middle = findMiddle(start, end);
	while (start != end)
	{
		if (middle->connection->fileDescriptor == fileDescriptor)
		{
			return middle->connection;
		}
		else if (middle->connection->fileDescriptor > fileDescriptor)
		{
			end = middle->previous;
			middle = findMiddle(start, end);
		}
		else
		{
			start = middle->next;
			middle = findMiddle(start, end);
		}
	}
	if (start->connection->fileDescriptor == fileDescriptor)
	{
		return start->connection;
	}
	else
	{
		return NULL;
	}
}

struct ClientConnection * findConnectionByTlsSession(struct ClientConnectionList * list, gnutls_session_t * tlsSession)
{
	
}

int removeConnectionByFileDescriptor(struct ClientConnectionList * list, int fileDescriptor)
{
	struct ClientConnectionNode * start = list->head, * end = list->tail, * middle = findMiddle(start, end), * toDelete = NULL;

	// Find the node that is to be deleted:
	while (start != end && toDelete == NULL)
	{
		if (middle->connection->fileDescriptor == fileDescriptor)
		{
			toDelete = middle;
		}
		else if (middle->connection->fileDescriptor > fileDescriptor)
		{
			end = middle->previous;
			middle = findMiddle(start, end);
		}
		else
		{
			start = middle->next;
			middle = findMiddle(start, end);
		}
	}
	if (start->connection->fileDescriptor == fileDescriptor)
	{
		toDelete = start;
	}

	if (toDelete == NULL)
	{
		return -1;
	}

	// Set the appropriate pointers on other nodes:
	if (toDelete->previous != NULL)
	{
		toDelete->previous->next = toDelete->next;
	}
	if (toDelete->next != NULL)
	{
		toDelete->next->previous = toDelete->previous;
	}

	// Set the appropriate pointers on the list:
	if (list->head == toDelete)
	{
		list->head = toDelete->next;
	}
	if (list->tail == toDelete)
	{
		list->tail = toDelete->previous;
	}

	
	list->clientCount--;
	
	// Free the connection:
	free(toDelete->connection->tlsSession);
	free(toDelete->connection);
	free(toDelete);

	return 0;
}

int addNewConnection(struct ClientConnectionList * list, int fileDescriptor, gnutls_session_t * tlsSession)
{
	// Allocate memory for the structures:
	struct ClientConnectionNode * newConnectionNode = calloc(1, sizeof(struct ClientConnectionNode));
	newConnectionNode->connection = calloc(1, sizeof(struct ClientConnection));
	
	// Set the appropriate data in the structure:
	newConnectionNode->next = NULL;
	newConnectionNode->previous = NULL;
	newConnectionNode->connection->tlsSession = tlsSession;
	newConnectionNode->connection->fileDescriptor = fileDescriptor;

	// If it's the first node in the list:
	if (list->head == NULL && list->tail == NULL)
	{
		list->head = newConnectionNode;
		list->tail = newConnectionNode;

		list->clientCount++;

		return 0;
	}

	// Insert it in the appropriate place in the list:
	else
	{
		struct ClientConnectionNode * currentNode = list->head;

		// Seek through the list until we find the appropriate spot to insert the new connection:
		while (currentNode->connection->fileDescriptor < fileDescriptor)
		{
			// If we've reached the end of the list:
			if  (currentNode->next == NULL)
			{
				currentNode->next = newConnectionNode;
				newConnectionNode->previous = currentNode;
				list->tail = newConnectionNode;
				
				list->clientCount++;

				return 0;
			}
			else
			{
				currentNode = currentNode->next;
			}
		}

		newConnectionNode->previous = currentNode->previous;
		newConnectionNode->next = currentNode;
		currentNode->previous = newConnectionNode;

		if (newConnectionNode->previous == NULL)
		{
			list->head = newConnectionNode;
		}
		if (newConnectionNode->next == NULL)
		{
			list->tail = newConnectionNode;
		}		
		list->clientCount++;

		return 0;
	}
}

// ===================================================
// | End of connections.c, copyright notice follows. |
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
