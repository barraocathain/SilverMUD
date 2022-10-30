// Hopefully optimized corestat to string:
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/playerdata.h"

void main(int argc, char ** argv)
{
	getCoreStatFromString(argv[1], strlen(argv[1]));
}
