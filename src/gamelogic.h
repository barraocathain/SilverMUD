// gamelogic.h: Function prototypes and data-structures for dealing with game logic.
// Barry Kane, 2022.
#ifndef GAMELOGIC_H
#define GAMELOGIC_H
#include "queue.h"
#include "areadata.h"
#include "constants.h"
#include "playerdata.h"
#include "inputoutput.h"

// ========================
// -=[ Data Structures ]=-:
// ========================

// An event for storing the information needed to evaluate a command:
typedef struct commandEvent commandEvent;
typedef struct commandEvent
{
	playerInfo * caller;
	commandEvent * next;
	char * command;
	char * arguments;
} commandEvent;

// A data-structure containing the needed parameters for the main game loop:
typedef struct gameLogicParameters
{
	// Players:
	int * playerCount;
	playerInfo * connectedPlayers;

	// Queues:
	queue * inputQueue;
	queue * outputQueue;

	// Lists:
	list * areaList;
	list * globalSkillList;
} gameLogicParameters;

// ========================
// -=[    Functions    ]=-:
// ========================

// Thread function which runs the main game loop, given the needed parameters:
void * gameLogicHandler(void * parameters);

// Enqueue a command that has been sent as a message from a user to a queue:
void queueMessagedCommand(queue * queue, inputMessage * messageToQueue);

// Evaluate the next commandEvent in a queue:
int evaluateNextCommand(gameLogicParameters * parameters, queue * queue);

// Enqueue a command to a queue:
void queueCommand(queue * queue, char * command, char * arguments, int commandLength, int argumentsLength,
				  playerInfo * callingPlayer);

// ============================
// -=[ Gameplay Primitives ]=-:
// ============================

// The possible outcomes of a check or challenge:
typedef enum outcome
{
	CRITICAL_FAILURE,
	FAILURE,
	SUCCESS,
	CRITICAL_SUCCESS,
	ERROR
} outcome;

// Move a player along a path in their current area:
int movePlayerToArea(playerInfo * player, char * requestedPath);

// Run a stat check for the given player, returning an outcome:
outcome statCheck(playerInfo * player, int chance, coreStat statToCheck);

// Run a skill check for the given player, returning an outcome:
outcome skillCheck(playerInfo * player, int chance, char * skillName, size_t skillNameLength, list * globalSkillList);

#endif
