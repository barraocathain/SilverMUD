// schemeintegration.h: Function definitions for SilverMUD's Scheme integration.
// Barra Ó Catháin, 2023.
#include <libguile.h>
#include "schemeintegration.h"

void * schemeHandler(void * parameters)
{
	scm_init_guile();
	scm_shell(0, NULL);
}

