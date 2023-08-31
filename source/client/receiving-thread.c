// ==========================================
// | SilverMUD Client - receiving-thread.c  |
// | Copyright (C) 2023, Barra Ó Catháin    |
// | See end of file for copyright notice.  |
// ==========================================
#include <ncurses.h>
#include <gnutls/gnutls.h>

#include "../messages.h"
#include "client-drawing.h"
#include "receiving-thread.h"

void * receivingThreadHandler(void * threadArguments)
{
	start_color();
	use_default_colors();
	init_pair(1, COLOR_GREEN,     -1);
	init_pair(2, COLOR_YELLOW,    -1);
	init_pair(3, COLOR_RED,       -1);
	init_pair(4, COLOR_BLUE,      -1);
	init_pair(5, COLOR_CYAN,      -1);
	init_pair(6, COLOR_MAGENTA,   -1);

	// Unpack the thread's arguments:
	gnutls_session_t session = ((struct ReceivingThreadArguments *)threadArguments)->session;
	WINDOW * chatWindow = ((struct ReceivingThreadArguments *)threadArguments)->chatWindow,
		* gameWindow = ((struct ReceivingThreadArguments *)threadArguments)->gameWindow,
		* inputWindow = ((struct ReceivingThreadArguments *)threadArguments)->inputWindow;
	
	// Print a message into the game window:
	wprintw(gameWindow, "Connection successful. Welcome to ");
	wattrset(gameWindow, COLOR_PAIR(2));
	wprintw(gameWindow, "SilverMUD!\n");
	wattrset(gameWindow, A_NORMAL);	

	struct ServerToClientMessage currentMessage;
	while (true)
	{
		gnutls_record_recv(session, &currentMessage, sizeof(struct ServerToClientMessage));
		wattrset(chatWindow, A_BOLD);
		wprintw(chatWindow, "<%s>: ", currentMessage.name);
		wattrset(chatWindow, A_NORMAL);
		wprintw(chatWindow, "%s\n", currentMessage.content);
		redrawClientLayout(gameWindow, chatWindow, inputWindow);
	}
}

// ========================================================
// | End of receiving-thread.c, copyright notice follows. |
// ========================================================

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
