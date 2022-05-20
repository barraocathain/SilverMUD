// gamelogic.h: Header file contatning function prototypes and datastructures
// for dealing with the game's logic.
// Barry Kane, 2022.
#ifndef GAMELOGIC_H
#define GAMELOGIC_H
#include "constants.h"
#include "playerdata.h"
#include "inputoutput.h"

// =======================
// -=[ Main Game Loop ]=-:
// =======================

// A datastructure containing the needed parameters for a main game loop:
typedef struct gameLogicParameters
{
	int * playerCount;
	playerInfo * connectedPlayers;
	inputMessageQueue * inputQueue;
	outputMessageQueue * outputQueue;
} gameLogicParameters;

// Thread function which runs the main game loop, given the needed parameters:
void * gameLogicLoop(void * parameters);

#endif
