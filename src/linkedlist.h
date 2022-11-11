// linkedlist.h: Defines the linked list datatype for SilverMUD.
// Barry Kane, 2022.
#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include "playerdata.h"
#include "areadata.h"

// ========================
// -=[ Data Structures ]=-:
// ========================

typedef enum listDataType
{
	PATH,
	AREA,
	PLAYER,
	SKILL
} listDataType;

typedef union listData
{
	playerPath * path;
	playerArea * area;
	playerInfo * player;
	playerSkill * skill;
} listData;
	
typedef struct listNode listNode;
typedef struct listNode
{
	listData data;
	listNode * next;
	listNode * previous;
} listNode;

typedef struct list
{
	listDataType type;
	size_t itemCount;
	listNode * head;
	listNode * tail;
} list;

// ==================
// -=[ Functions ]=-:
// ==================

// Allocates and instantiates a list of the specified type:
list * createList(listDataType type);
 
// Deallocates a list and all of it's members:
int destroyList(list ** list);

// Returns the data at a given index in a list:
listData * getFromList(list * list, size_t listIndex);

// Returns the node at a given index in a list:
listNode * getNodeFromList(list * list, size_t listIndex);
	
// Adds the given data to the end of a list:
listNode * addToList(list * list, void * data, listDataType type);

// Insert the given data at a given index in the list:
listNode * insertIntoList(list * list, void * data, listDataType type, size_t listIndex);

// Delete the given data from a list:
bool deleteFromList(list * list, void * data, listDataType type);

// Delete the data from a given point in a list:
int removeFromList(list * list, listDataType type, size_t listIndex);

// Get the index of a given piece of data in a list:
bool getIndexFromList(list * list, void * data, listDataType type, size_t * index);

#endif
