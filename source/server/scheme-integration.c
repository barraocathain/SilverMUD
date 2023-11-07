// ===========================================
// | SilverMUD Server - scheme-integration.c |
// | Copyright (C) 2023, Barra Ó Catháin     |
// | See end of file for copyright notice.   |
// ===========================================
#include <stdbool.h>
#include <libguile.h>

#include "../messages.h"
#include "output-queue.h"

SCM scheme_get_player_by_name(SCM name, SCM queue);

SCM push_output_message(SCM queue, SCM deallocate_list, SCM recepients, SCM type, SCM name, SCM content)
{
	// Convert our scheme values into appropriate data types:
	struct OutputQueue * queue_c = scm_to_pointer(queue);
	bool deallocate_list_c = scm_to_bool(deallocate_list);
	struct PlayerList * recepients_c = scm_to_pointer(recepients);
	enum MessageTypes type_c = scm_to_int(type);

	// Turn the Scheme strings into C strings:
	size_t nameLength, contentLength;
	char * name_c = scm_to_locale_stringn(name, &nameLength);
	char * content_c = scm_to_locale_stringn(content, &contentLength);

	// Call the C function:
	pushOutputMessage(queue_c, deallocate_list_c, recepients_c, type_c, name_c, content_c,
					  nameLength, contentLength);

	// Free the created C strings:
	free(name_c);
	free(content_c);

	return SCM_BOOL_T;
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
