// ==========================================
// | SilverMUD Client - receiving-thread.h  |
// | Copyright (C) 2023, Barra Ó Catháin    |
// | See end of file for copyright notice.  |
// ==========================================
#ifndef RECEIVING_THREAD_H
#define RECEIVING_THREAD_H
#include <ncurses.h>
#include <gnutls/gnutls.h>

struct ReceivingThreadArguments
{
	WINDOW * chatWindow, * gameWindow, * inputWindow;
	gnutls_session_t session;	
};

void * receivingThreadHandler(void * threadArguments);

#endif
// ========================================================
// | End of receiving-thread.h, copyright notice follows. |
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
