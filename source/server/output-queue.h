// =========================================
// | SilverMUD Server - output-queue.h     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef OUTPUT_QUEUE_H
#define OUTPUT_QUEUE_H
#include <stdbool.h>
#include <pthread.h>

#include "../messages.h"

struct OutputMessage
{
	// Allows for easy reuse of existing player lists, such as the global list
	// or an area's playerlist:	
	bool deallocatePlayerList;
	
	struct OutputMessage * next;
	struct PlayerList * recepients;
	struct ServerToClientMessage * message;
};

struct OutputQueue
{
	pthread_mutex_t mutex;
	size_t count;
	struct OutputMessage * front, * back;	
};

struct OutputQueue * const createOutputQueue();

size_t pushOutputMessage(struct OutputQueue * const queue,
						 const bool deallocatePlayerList,
						 struct PlayerList * const recepients,
						 const enum MessageTypes type,
						 const char const * name, const char const * content,
						 const size_t nameLength, const size_t contentLength);
	
struct OutputMessage * popOutputMessage();

void deallocateOutputMessage(struct OutputMessage ** message);

#endif
// ====================================================
// | End of output-queue.h, copyright notice follows. |
// ====================================================

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

