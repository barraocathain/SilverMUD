// inputhandling.h: Header file for the inputhandling library for SilverMUD.
// Barry Kane, 2021
#ifndef INPUTHANDLING_H
#define INPUTHANDLING_H
#include <ctype.h>

// Sanatize user input to ensure it's okay to send to the server:
void userInputSanatize(char * inputString, int length);

#endif
