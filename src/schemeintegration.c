// schemeintegration.h: Function definitions for SilverMUD's Scheme integration.
// Barra Ó Catháin, 2023.
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

// Message every currently connected player from Scheme:
SCM scheme_message_everyone(SCM sendername, SCM messagecontent, SCM outputqueue)
{
	// Allocate the memory for the needed data structures:
	outputMessage * newOutputMessage = calloc(1, sizeof(userMessage));
	userMessage * newMessage = calloc(1, sizeof(userMessage));

	// Set some basic information for the output message, allowing it to be sent to everyone:
	newOutputMessage->content = newMessage;
	newOutputMessage->recipientsCount = 0;
	newOutputMessage->recipients = NULL;

	// Convert the Scheme strings to C strings, and ensure they're NULL terminated:
	scm_to_locale_stringbuf(sendername, newMessage->senderName, 31);
	newMessage->senderName[31] = '\0';
	scm_to_locale_stringbuf(messagecontent, newMessage->messageContent, MAX - 1);
	newMessage->messageContent[MAX - 1] = '\0';

	// Clean up the message contents to ensure they're safe to send and display correctly:
	userNameSanatize(newMessage->senderName, 32);
	userInputSanatize(newMessage->messageContent, MAX);

	// Push it to the queue, where it will be handled and de-allocated:
	pushQueue(scm_to_pointer(outputqueue), newOutputMessage, OUTPUT_MESSAGE);

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
	scm_c_define_gsubr("create-skill", 2, 0, 0, &scheme_create_skill);
	scm_c_define_gsubr("message-everyone", 3, 0, 0, &scheme_message_everyone);

	// Define the various game state pointers as Scheme objects:
	scm_c_define("skill-list", scm_from_pointer(schemeThreadParameters->skillList, NULL));
	scm_c_define("output-queue", scm_from_pointer(schemeThreadParameters->outputQueue, NULL));

	// Drop into the Scheme interpreter:
	scm_shell(0, NULL);

	// Return NULL. This should be unreachable.
	return NULL;
}
		
