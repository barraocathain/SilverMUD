// playerdata.h: Header file containing data structures for player data and function
// definitions for interacting with said data.
// Barry Kane, 2021.

typedef struct userMessage
{
	char senderName[32];
	char messageContent[1024];
} userMessage;

typedef struct playerInfo
{
	char playerName[32];
} playerInfo;



