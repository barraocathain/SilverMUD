// linkedlist.h: Defines the linked list datatype for SilverMUD.
// Barry Kane, 2022.
#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include "areadata.h"

// Let the compiler know there will be structs defined elsewhere:
typedef struct playerPath playerPath;
typedef struct playerArea playerArea;
typedef struct playerInfo playerInfo;
typedef struct playerSkill playerSkill;

// ========================
// -=[ Data Structures ]=-:
// ========================

// An enum of the possible data types that can be stored in a list:
typedef enum listDataType
{
	PATH,
	AREA,
	PLAYER,
	SKILL
} listDataType;

// A union containing a pointers to all data types that can be stored in a list:
typedef union listData
{
	playerPath * path;
	playerArea * area;
	playerInfo * player;
	playerSkill * skill;
} listData;

// A doubly linked node for the linked list type:
typedef struct listNode listNode;
typedef struct listNode
{
	listData data;
	listNode * next;
	listNode * previous;
} listNode;

// A header structure for the list containing the length, head, tail, and type of the list.
typedef struct list
{
	listDataType type;
	size_t itemCount;
	listNode * head;
	listNode * tail;
} list;

// ========================
// -=[    Functions    ]=-:
// ========================

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
