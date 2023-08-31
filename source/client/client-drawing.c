// ==========================================
// | SilverMUD Client - client-drawing.c    |
// | Copyright (C) 2023, Barra Ó Catháin    |
// | See end of file for copyright notice.  |
// ==========================================
#include "../config.h"
#include "client-drawing.h"

void redrawClientLayout(WINDOW * gameWindow, WINDOW * chatWindow, WINDOW * inputWindow)
{
	int height, width;
	getmaxyx(stdscr, height, width);
	
	// Draw the lines that will seperate windows:
	wborder(stdscr, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	attron(A_REVERSE);
	mvwhline(stdscr, 0, 0, '=', width);
	mvwhline(stdscr, height / 2, 0, '=', width);
	mvwhline(stdscr, height - 2, 0, '=', width);

	// Write the labels for windows:
	attron(COLOR_PAIR(1));
	mvwprintw(stdscr, 0, 1, " SilverMUD | Version %s ", PACKAGE_VERSION);
	mvwprintw(stdscr, height / 2, 1, " Chat ");
	mvwprintw(stdscr, height - 2, 1, " Input ");
	attroff(COLOR_PAIR(1));
	attroff(A_REVERSE);

	// Move the windows into place:
	mvwin(gameWindow, 1, 1);
	mvwin(chatWindow, (height / 2) + 1 , 1);
	mvwin(inputWindow, height - 1, 1);

	// Resize the windows:
	wresize(gameWindow, (height - 2) / 2, width - 2);
	wresize(chatWindow, ((height - 4) / 2) - (1 - (height % 2)), width - 2);
	wresize(inputWindow, 1, width - 2);
	
	// Refresh every window:
	wrefresh(stdscr);
	wrefresh(gameWindow);
	wrefresh(chatWindow);
	wrefresh(inputWindow);	
}

// ========================================================
// | End of client-drawing.c, copyright notice follows.   |
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
