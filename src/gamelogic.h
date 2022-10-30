// gamelogic.h: Header file contatning function prototypes and datastructures
// for dealing with the game's logic.
// Barry Kane, 2022.
#ifndef GAMELOGIC_H
#define GAMELOGIC_H
#include "areadata.h"
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
	areaNode * areaList;
	playerInfo * connectedPlayers;
	inputMessageQueue * inputQueue;
	outputMessageQueue * outputQueue;
	skillList * globalSkillList;
} gameLogicParameters;

// Thread function which runs the main game loop, given the needed parameters:
void * gameLogicLoop(void * parameters);

// ======================
// -=[ Command Queue ]=-:
// ======================
typedef struct commandEvent commandEvent;
typedef struct commandEvent
{
	playerInfo * caller;
	commandEvent * next;
	char * command;
	char * arguments;
} commandEvent;

// A first-in first-out queue for message input from players:
typedef struct commandQueue
{
	bool lock;
	bool paused;
	int currentLength;
	commandEvent * back;
	commandEvent * front;
} commandQueue;

// Create a commandQueue:
commandQueue * createCommandQueue(void);

// Enqueue a command to a commandQueue:
int queueCommand(commandQueue * queue, char * command, char * arguments,
				 int commandLength, int argumentsLength , playerInfo * callingPlayer);

// Enqueue a messaged command to a commandQueue:
int queueMessagedCommand(commandQueue * queue, inputMessage * messageToQueue);

// Dequeue the front commandEvent from a commandQueue:
int dequeueCommand(commandQueue * queue);

// Return the front commandEvent from a commandQueue:
commandEvent * peekCommand(commandQueue * queue); 

// Evaluate the next commandEvent:
int evaluateNextCommand(gameLogicParameters * parameters, commandQueue * queue);

/* // Evaluate the given commandEvent: */
/* int evaluateCommand(gameLogicParameters * parameters, commandEvent * command); */

// ============================
// -=[ Gameplay Primitives ]=-:
// ============================

// Player movement:
int movePlayerToArea(playerInfo * player, char * requestedPath);

typedef enum outcome
{
	CRITICAL_FAILURE,
	FAILURE,
	SUCCESS,
	CRITICAL_SUCCESS,
	ERROR
} outcome;

// Run a stat check:
outcome statCheck(playerInfo * player, int chance, coreStat statToCheck);

// Run a skill check:
outcome skillCheck(playerInfo * player, int chance, char * skillName, size_t skillNameLength, skillList * globalSkillList);

#endif
