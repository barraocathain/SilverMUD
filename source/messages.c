// =========================================
// | SilverMUD - messages.c                |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdlib.h>
#include <string.h>
#include <libguile.h>

#include "messages.h"

// Allocate and initialize a client to server message using the passed values:
struct ClientToServerMessage * createClientToServerMessage(char * content)
{
	// Allocate the needed memory for the message:
	struct ClientToServerMessage * newMessage = calloc(1, sizeof(struct ClientToServerMessage));

	// Copy the string and terminate it:
	strncpy(newMessage->content, content, MESSAGE_CONTENT_LENGTH - 1);
	newMessage->content[MESSAGE_CONTENT_LENGTH - 1] = '\0';

	// Return the pointer:
	return newMessage;
}

// A Scheme wrapper for creating client to server messages:
SCM scheme_createClientToServerMessage(SCM content)
{
	// Check that we have been provided the right Scheme type:
	if (scm_string_p(content))
	{
		// Convert the Scheme string to a C string:
		char * contentString = scm_to_locale_stringn(content, NULL);

		// Create the message:
		struct ClientToServerMessage * message = createClientToServerMessage(contentString);

		// Free the converted string:
		free(contentString);

		// Return the pointer as a Scheme object:
		return scm_from_pointer(message, NULL);
	}
	else
	{
		return SCM_BOOL_F;
	}
}

// Allocate and initialize a server to client message using the passed values:
struct ServerToClientMessage * createServerToClientMessage(uint8_t type, char * name, char * content)
{
	// Allocate the needed memory for the message:
	struct ServerToClientMessage * newMessage = calloc(1, sizeof(struct ServerToClientMessage));

	// Copy the type:
	newMessage->type = type;
	
	// Copy the strings and terminate them:
	strncpy(newMessage->name, name, MESSAGE_NAME_LENGTH - 1);
	newMessage->name[MESSAGE_NAME_LENGTH - 1] = '\0';
	strncpy(newMessage->content, content, MESSAGE_CONTENT_LENGTH - 1);
	newMessage->content[MESSAGE_CONTENT_LENGTH - 1] = '\0';

	return newMessage;
}

// A Scheme wrapper for creating server to client messages:
SCM scheme_createServerToClientMessage(SCM type, SCM name, SCM content)
{
	// Check that we have been provided the right Scheme type:
	if (scm_integer_p(type) && scm_string_p(name) && scm_string_p(content))
	{
		// Convert the integer to a C integer:
		uint8_t typeInteger = scm_to_uint8(type);
		
		// Convert the Scheme strings to C strings:
		char * nameString = scm_to_locale_stringn(name, NULL);
		char * contentString = scm_to_locale_stringn(content, NULL);

		// Create the message:
		struct ServerToClientMessage * message =
			createServerToClientMessage(typeInteger, nameString, contentString);

		// Free the converted string:
		free(nameString);
		free(contentString);

		// Return the pointer as a Scheme object:
		return scm_from_pointer(message, NULL);
	}
	else
	{
		return SCM_BOOL_F;
	}
}

// ================================================
// | End of messages.c, copyright notice follows. |
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
