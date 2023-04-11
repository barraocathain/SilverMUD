// schemeintegration.h: Function definitions for SilverMUD's Scheme integration.
// Barra Ó Catháin, 2023.
#include <string.h>
#include <libguile.h>
#include "schemeintegration.h"

// Create a player skill and add it to a given skill list from Scheme:
SCM scheme_create_skill(SCM string, SCM skilllist)
{	
	size_t skillNameLength = 0;
	char * skillName = scm_to_latin1_stringn(string, &skillNameLength);   
	createSkill(scm_to_pointer(skilllist), skillName, skillNameLength, false);
	free(skillName);
	return SCM_BOOL_T;
}

// Create a new area and add it to the list, given a name and area description, and an area list from Scheme.
// Returns the index of the new area:
SCM scheme_create_area(SCM area_name, SCM area_description, SCM area_list)
{
	// Check if the area list exists:
	list * areaList = scm_to_pointer(area_list);
	if (areaList == NULL || areaList->type != AREA)
	{
		return SCM_BOOL_F;
	}

	// Turn the SCM values into C strings that we can use:
	char * areaName = scm_to_locale_stringn(area_name, NULL);
	char * areaDescription= scm_to_locale_stringn(area_description, NULL);

	// Create and add the area to the area list:
	addToList(areaList, createArea(areaName, areaDescription), AREA);

	// The new index of the area will be at the end of the list. Consider returning a pointer to the area:
	size_t areaIndex = areaList->itemCount - 1;

	// Free the strings:
	free(areaName);
	free(areaDescription);

	// Return the area index to Scheme for further lispy hacking:
	return scm_from_size_t(areaIndex);
}

// Create a one way path between two areas from scheme:
SCM scheme_create_path(SCM path_name, SCM from_area_index, SCM to_area_index, SCM area_list)
{
	// Check if the area list exists:
	list * areaList = scm_to_pointer(area_list);
	if (areaList == NULL || areaList->type != AREA)
	{
		return SCM_BOOL_F;
	}

	// Check if the areas exist:
	playerArea * fromArea = getFromList(areaList, scm_to_size_t(from_area_index))->area;
	playerArea * toArea = getFromList(areaList, scm_to_size_t(to_area_index))->area;

	if (fromArea == NULL || toArea == NULL)
	{
		return SCM_BOOL_F;
	}
		
	// Turn the SCM value into a C string that we can use:
	char * pathName = scm_to_locale_stringn(path_name, NULL);

	// Create the path:
	createOneWayPath(fromArea, toArea, pathName);

	// Free the string:
	free(pathName);

	// Return true to Scheme:
	return SCM_BOOL_T;
}

// Change the name of an existing area in a list, given the number of the area in the list, from Scheme:
SCM scheme_change_area_name(SCM new_name, SCM area_number, SCM area_list)
{
	// Check if the area exists:
	list * areaList = scm_to_pointer(area_list);
	size_t areaNumber = scm_to_size_t(area_number);
	
	if (areaList->type != AREA)
	{
		return SCM_BOOL_F;
	}
	
	playerArea * area = getFromList(areaList, areaNumber)->area;

	if (area == NULL)
	{
		return SCM_BOOL_F;
	}

	// Create a string from the Scheme string and copy it into the area:
	size_t newNameLength = 0;
	char * newName = scm_to_locale_stringn(new_name, &newNameLength);
	memset(area->areaName, 0, 32);
	if (newNameLength > 32)
	{
		memcpy(area->areaName, newName, 31);
		area->areaName[31] = '\0';
	}
	else
	{
		memcpy(area->areaName, newName, newNameLength);
		area->areaName[31] = '\0';
	}

	free(newName);
	
	return SCM_BOOL_T;
}

// Change the description of an existing area in a list, given the number of the area in the list, from Scheme:
SCM scheme_change_area_description(SCM new_description, SCM area_number, SCM area_list)
{
	// Check if the area exists:
	list * areaList = scm_to_pointer(area_list);
	size_t areaNumber = scm_to_size_t(area_number);

	if (areaList->type != AREA)
	{
		return SCM_BOOL_F;
	}
	
	playerArea * area = getFromList(areaList, areaNumber)->area;

	if (area == NULL)
	{
		return SCM_BOOL_F;
	}

	// Create a string from the Scheme string and copy it into the area:
	size_t newDescriptionLength = 0;
	char * newDescription = scm_to_locale_stringn(new_description, &newDescriptionLength);
	memset(area->areaDescription, 0, MAX - 35);
	if (newDescriptionLength > MAX - 35)
	{
		memcpy(area->areaDescription, newDescription, MAX - 35);
		area->areaDescription[MAX - 36] = '\0';
	}
	else
	{
		memcpy(area->areaDescription, newDescription, newDescriptionLength);
		area->areaDescription[MAX - 36] = '\0';
	}

	free(newDescription);
	
	return SCM_BOOL_T;
}

// Message every currently connected player from Scheme:
SCM scheme_message_everyone(SCM sender_name, SCM message_content, SCM output_queue)
{
	// Allocate the memory for the needed data structures:
	outputMessage * newOutputMessage = calloc(1, sizeof(userMessage));
	userMessage * newMessage = calloc(1, sizeof(userMessage));

	// Set some basic information for the output message, allowing it to be sent to everyone:
	newOutputMessage->content = newMessage;
	newOutputMessage->recipientsCount = 0;
	newOutputMessage->recipients = NULL;

	// Convert the Scheme strings to C strings, and ensure they're NULL terminated:
	scm_to_locale_stringbuf(sender_name, newMessage->senderName, 31);
	newMessage->senderName[31] = '\0';
	scm_to_locale_stringbuf(message_content, newMessage->messageContent, MAX - 1);
	newMessage->messageContent[MAX - 1] = '\0';

	// Clean up the message contents to ensure they're safe to send and display correctly:
	userNameSanatize(newMessage->senderName, 32);
	userInputSanatize(newMessage->messageContent, MAX);

	// Push it to the queue, where it will be handled and de-allocated:
	pushQueue(scm_to_pointer(output_queue), newOutputMessage, OUTPUT_MESSAGE);

	// Return a Scheme #t value:
	return SCM_BOOL_T;
}

// The function ran by the Scheme thread which handles all game-master and interpreter interaction:
void * schemeHandler(void * parameters)
{
	// Take in the needed values from the main thread and make it back into the struct:
	SchemeThreadParameters * schemeThreadParameters = parameters;

	// Initialize GNU Guile:
   	scm_init_guile();

	// Register the various functions:
	scm_c_define_gsubr("create-area", 3, 0, 0, &scheme_create_area);
	scm_c_define_gsubr("create-path", 4, 0, 0, &scheme_create_path);
	scm_c_define_gsubr("create-skill", 2, 0, 0, &scheme_create_skill);
	scm_c_define_gsubr("message-everyone", 3, 0, 0, &scheme_message_everyone);
	scm_c_define_gsubr("change-area-name", 3, 0, 0, &scheme_change_area_name);
	scm_c_define_gsubr("change-area-description", 3, 0, 0, &scheme_change_area_description);
	
	// Define the various game state pointers as Scheme objects:
	scm_c_define("area-list", scm_from_pointer(schemeThreadParameters->areaList, NULL));
	scm_c_define("skill-list", scm_from_pointer(schemeThreadParameters->skillList, NULL));
	scm_c_define("output-queue", scm_from_pointer(schemeThreadParameters->outputQueue, NULL));

	// Drop into the Scheme interpreter:
	scm_shell(0, NULL);

	// Return NULL. This should be unreachable.
	return NULL;
}
		
