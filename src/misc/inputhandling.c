// inputhandling.c: Implementation of input handling library for SilverMUD.
// Barry Kane, 2021.
#include <ctype.h>

void userInputSanatize(char * inputString, int length)
{
	for(int index = 0; index <= length; index++)
	{
		if(!isprint(inputString[index]))
		{
			inputString[index] = '\n';
			inputString[index + 1] = '\0';
			break;
		}
	}
}
