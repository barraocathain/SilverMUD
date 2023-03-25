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

void * schemeHandler(void * parameters)
{
	SchemeThreadParameters * schemeThreadParameters = parameters;
	
   	scm_init_guile();
	scm_c_define_gsubr("create-skill", 2, 0, 0, &scheme_create_skill);
	scm_c_define("skill-list", scm_from_pointer(schemeThreadParameters->skillList, NULL));
	scm_shell(0, NULL);
}
		
