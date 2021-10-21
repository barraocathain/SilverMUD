// playerdata.h: Header file containing data structures for player data and function
// definitions for interacting with said data.
// Barry Kane, 2021.

typedef struct userMessage
{
	char senderName[32];
	char messageContent[1024];

} userMessage;

typedef struct playerPath playerPath;
typedef struct playerArea playerArea;

struct playerPath
{
	char pathName[32];
	playerArea * areaToJoin;
};

struct playerArea
{
	char areaName[32];
        playerPath * areaExits[32];
};

typedef struct playerInfo
{
	char playerName[32];
	playerArea * currentArea;
} playerInfo;

// Move a player to a different area given a path in the area.
int movePlayerToArea(playerInfo * player, char * requestedPath);
