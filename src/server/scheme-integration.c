// ===========================================
// | SilverMUD Server - scheme-integration.c |
// | Copyright (C) 2023, Barra Ó Catháin     |
// | See end of file for copyright notice.   |
// ===========================================
#include <libguile.h>

#include "scheme-integration.h"

// The function ran by the Scheme thread which runs a text-based REPL:
void * schemeREPLHandler (void * threadParameters)
{
	// Initialize Scheme:
	scm_init_guile();

	// Enable Readline support:
	scm_c_eval_string("(begin (use-modules (ice-9 readline)) (activate-readline))");

	// Start a Scheme REPL:
	scm_shell(0, NULL);
	
	// Return NULL to the calling thread:
	return NULL;
}

// ==========================================================
// | End of scheme-integration.c, copyright notice follows. |
// ==========================================================

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
