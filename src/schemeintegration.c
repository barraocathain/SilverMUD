// schemeintegration.h: Function definitions for SilverMUD's Scheme integration.
// Barra Ó Catháin, 2023.
#include <libguile.h>
#include "schemeintegration.h"

SCM scheme_create_skill(SCM string, SCM skilllist)
{
	size_t skillNameLength = 0;
	char * skillName = scm_to_latin1_stringn(string, &skillNameLength);   
	createSkill(scm_to_pointer(skilllist), skillName, skillNameLength, false);
	free(skillName);
	return SCM_BOOL_T;
}

SCM scheme_message_everyone(SCM sendername, SCM messagecontent, SCM outputqueue)
{
	outputMessage * newOutputMessage = calloc(1, sizeof(userMessage));
	userMessage * newMessage = calloc(1, sizeof(userMessage));
	newOutputMessage->content = newMessage;
	newOutputMessage->recipientsCount = 0;
	newOutputMessage->recipients = NULL;

	scm_to_locale_stringbuf(sendername, newMessage->senderName, 31);
	newMessage->senderName[31] = '\0';
	scm_to_locale_stringbuf(messagecontent, newMessage->messageContent, MAX - 1);
	newMessage->messageContent[MAX - 1] = '\0';

	userNameSanatize(newMessage->senderName, 32);
	userInputSanatize(newMessage->messageContent, MAX);
	pushQueue(scm_to_pointer(outputqueue), newOutputMessage, OUTPUT_MESSAGE);
	return SCM_BOOL_T;
}
void * schemeHandler(void * parameters)
{
	SchemeThreadParameters * schemeThreadParameters = parameters;
	
   	scm_init_guile();
	scm_c_define_gsubr("create-skill", 2, 0, 0, &scheme_create_skill);
	scm_c_define_gsubr("message-everyone", 3, 0, 0, &scheme_message_everyone);
	scm_c_define("skill-list", scm_from_pointer(schemeThreadParameters->skillList, NULL));
	scm_c_define("output-queue", scm_from_pointer(schemeThreadParameters->outputQueue, NULL));
	scm_shell(0, NULL);
}
		
