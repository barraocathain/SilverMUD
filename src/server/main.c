// =========================================
// | SilverMUD Server - main.c             |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdio.h>
#include <pthread.h>

#include "scheme-integration.h"

int main (int argc, char ** argv)
{
	// Print a welcome message:
	printf("SilverMUD Server - Starting Now.\n"
		   "================================\n");

	// Create the Scheme thread:
	pthread_t schemeThread;
	pthread_create(&schemeThread, NULL, schemeThreadHandler, NULL);

	// Wait for all other threads to terminate:
	pthread_join(schemeThread, NULL);
	
	// Return a successful status code to the operating system:
	return 0;
}

// ============================================
// | End of main.c, copyright notice follows. |
// ============================================

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
