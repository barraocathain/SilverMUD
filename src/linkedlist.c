// linkedlist.h: Function definitions for the list type for SilverMUD.
// Barry Kane, 2022.
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "playerdata.h"
#include "linkedlist.h"

// Deallocate a given list node, including it's data:
static inline void deallocateListNode(listNode * node, listDataType type)
{
	// Delete the node:
	switch(type)
	{
		case PLAYER:
		{
			deallocatePlayer(node->data.player);
			break;
		}
		case AREA:
		{
			destroyList(&(node->data.area->pathList));
			free(node->data.area);
			free(node);
			break;
		}
		case PATH:
		{
			free(node->data.path);
			free(node);
			break;
		}
		case SKILL:
		{
			free(node->data.skill);
			free(node);
			break;
		}
	}
}

// Allocates and instantiates a list of the specified type:
list * createList(listDataType type)
{
	// Allocate and clear the memory for the list:
	list * newList = calloc(sizeof(list), 1);

	// Set the appropriate values in the new list:
	newList->type = type;
	newList->itemCount = 0;
	newList->head = NULL;
	newList->tail = NULL;

	// Return the new list:
	return newList;
}

// Deallocates a list and all of it's members:
int destroyList(list ** list)
{
	// Check if the list is empty:
	if((*list)->itemCount == 0)
	{
		free(*list);
		list = NULL;
		return 0;
	}
	else
	{
		while((*list)->itemCount > 0)
		{
			removeFromList((*list), (*list)->type, (*list)->itemCount - 1);
		}
		free(*list);
		*list = NULL;
		return 0;
	}
}

// Returns the data at a given index in a list:
listData * getFromList(list * list, size_t listIndex)
{
	// Check that we were given a valid index:
	if(listIndex > (list->itemCount - 1))
	{
		perror("Invalid index specified.\n");
		return NULL;
	}
	// Return the head if index is 0:
	else if(listIndex == 0)
	{
		return &(list->head->data);
	}
	// Loop through the entries in the list until we get to the right one:
	else
	{
		listNode * currentNode = list->head;
		while(listIndex-- > 0)
		{
			currentNode = currentNode->next;
		}
		return &(currentNode->data);
	}
}

// Returns the node at a given index in a list:
listNode * getNodeFromList(list * list, size_t listIndex)
{
	// Check that we were given a valid index:
	if(listIndex > (list->itemCount - 1))
	{
		perror("Invalid index specified.\n");
		return NULL;
	}
	// Return the head if index is 0:
	else if(listIndex == 0)
	{
		return list->head;
	}
	// Loop through the entries in the list until we get to the right one:
	else
	{
		listNode * currentNode = list->head;
		while(listIndex-- > 0)
		{
			currentNode = currentNode->next;
		}
		return currentNode;
	}
}

// Adds the given data to the end of a list:
listNode * addToList(list * list, void * data, listDataType type)
{
	// Check the type:
	if(type != list->type)
	{
		fprintf(stderr, "Not the correct type for this list.\n");
		return NULL;
	}

	// If this is the first item in the list:
	if(list->itemCount == 0)
	{
		// Allocate the new node for the list:
		list->head = calloc(1, sizeof(listNode));
		
		// Set the appropriate pointers for the list:
		list->head->next = NULL;
		list->head->previous = NULL;
		list->tail = list->head;

		// Add the data to the new node:
		switch(type)
		{
			case PATH:
			{
				list->head->data.path = (playerPath *)data;
				break;
			}
			case AREA:
			{
				list->head->data.area = (playerArea *)data;
				break;
			}
			case PLAYER:
			{
				list->head->data.player = (playerInfo *)data;
				break;
			}
			case SKILL:
			{
				list->head->data.skill = (playerSkill *)data;
				break;
			}
		}
	}
	else
	{	  
		// Allocate the new node at the end of the list:
		list->tail->next = calloc(1, sizeof(listNode));
		
		// Add the data to the new node:
		switch(type)
		{
			case PATH:
			{
				list->tail->next->data.path = (playerPath *)data;
				break;
			}
			case AREA:
			{
				list->tail->next->data.area = (playerArea *)data;
				break;
			}
			case PLAYER:
			{
				list->tail->next->data.player = (playerInfo *)data;
				break;
			}
			case SKILL:
			{
				list->tail->next->data.skill = (playerSkill *)data;
				break;
			}
		}
		
		// Set the appropriate pointers in the new node:
		list->tail->next->previous = list->tail;
		
		// Set the list's tail to the new tail:
		list->tail = list->tail->next;
	}
    // Increase the count of items in the list:
	list->itemCount++;

	// Return the new item in the list:
	return list->tail;
}

// Insert the given data at a given index in the list:
listNode * insertIntoList(list * list, void * data, listDataType type, size_t listIndex)
{
	// Check that the types are correct:
	if(list->type != type)
	{
		fprintf(stderr, "Types do not match.\n");
		return NULL;
	}
	
	// Handle the special case of adding to the end of the list:
	if(listIndex == (list->itemCount - 1))
	{
		return addToList(list, data, type);
	}

	// Handle the special case of adding to the beginning of the list:
	if(listIndex == 0)
	{
		// Create the new node:
		listNode * newNode = calloc(1, sizeof(listNode));

		// Add the data to the node:
		switch(type)
		{
			case PATH:
			{
				newNode->data.path = (playerPath *)data;
				break;
			}
			case AREA:
			{
				newNode->data.area = (playerArea *)data;
				break;
			}
			case PLAYER:
			{
				newNode->data.player = (playerInfo *)data;
				break;
			}
			case SKILL:
			{
				newNode->data.skill = (playerSkill *)data;
				break;
			}
		}
		
		// Place it in the list:
		newNode->next = list->head;
		newNode->previous = NULL;
		list->head->previous = newNode;
		list->head = newNode;
		list->itemCount++;

		// Return the node:
		return newNode;
	}
	
	// Check that the index is valid:
	if(listIndex > (list->itemCount - 1))
	{
		fprintf(stderr, "Index is invalid for the list.\n");
		return NULL;
	}
	
	// Get the current node at the index:
	listNode * currentNode = list->head;
	for(size_t index = 0; index < listIndex; index++)
	{
		currentNode = currentNode->next;
	}

	// Get the node before the current node:
	listNode * previousNode = currentNode->previous;

	// Create the new node:
	previousNode->next = calloc(1, sizeof(listNode));
	currentNode->previous = previousNode->next;
	previousNode->next->next = currentNode;
	previousNode->next->previous = previousNode;
	
	// Add the data to the node:
	switch(type)
	{
		case PATH:
		{
			previousNode->next->data.path = (playerPath *)data;
			break;
		}
		case AREA:
		{
			previousNode->next->data.area = (playerArea *)data;
			break;
		}
		case PLAYER:
		{
			previousNode->next->data.player = (playerInfo *)data;
			break;
		}
		case SKILL:
		{
			previousNode->next->data.skill = (playerSkill *)data;
			break;
		}
	}
	list->itemCount++;
	return previousNode->next;
}

// Delete the given data from a list:
bool deleteFromList(list * list, void * data, listDataType type)
{
	size_t index = 0;
	if(getIndexFromList(list, data, type, &index) == false)
	{
		return false;
	}
	else
	{
		removeFromList(list, type, index);
		return true;
	}
}

// Delete the data from a given point in a list:
int removeFromList(list * list, listDataType type, size_t listIndex)
{
	// Check that we're removing the correct type:
	if(list->type != type)
	{
		return -1;
	}
	
	// Check the list index is valid:
	if(listIndex > list->itemCount - 1)
	{
		return -2;
	}

	// The first node in the list:
	if(listIndex == 0)
	{		
		// Get the current head and move the list's head on:
		listNode * oldHead = list->head;
		list->head = list->head->next;
		
		// If we haven't removed the last item, set the previous pointer
		// in the new head to null.
		if(list->head != NULL)
		{
			list->head->previous = NULL;
		}

		// Delete the node:
		deallocateListNode(oldHead, type);
		
		// Return the new amount of items in the list:
		list->itemCount--;
		return list->itemCount;
	}
	// The last node in the list:
	else if(listIndex == (list->itemCount - 1))
	{
		// Move the tail up by one:
		list->tail = list->tail->previous;

		// Deallocate the former tail:
		deallocateListNode(list->tail->next, type);

		// Set the appropriate pointer:
		list->tail->next = NULL;

		// Return the new amount of items in the list:
		list->itemCount--;
		return list->itemCount;
	}
	// A node in the middle of the list:
	else 
	{
		// Get the needed node as a pointer:
		listNode * nodeToDelete = getNodeFromList(list, listIndex);

		// Set the appropriate pointers for the surrounding nodes:
		nodeToDelete->previous->next = nodeToDelete->next;
		nodeToDelete->next->previous = nodeToDelete->previous;

		// Deallocate the node:
		deallocateListNode(nodeToDelete, type);
		
		// Return the new amount of items in the list:
		list->itemCount--;
		return list->itemCount;
	}
}

// Get the index of a given piece of data in a list:
bool getIndexFromList(list * list, void * data, listDataType type, size_t * index)
{
	// Check the list types are the same:
	if(list->type == type)
	{
		fprintf(stderr, "List types do not match.\n");
		return false;
	}

	for(*index = 0; *index < list->itemCount; *index += 1)
	{
		switch(type)
		{
			case AREA:
			{
				if(getFromList(list, *index)->area == data)
				{
					return true;					
				}
				break;
			}
			case PLAYER:
			{
				if(getFromList(list, *index)->player == data)
				{
					return true;					
				}
				break;				
			}
			case PATH:
			{
				if(getFromList(list, *index)->path == data)
				{
					return true;					
				}
				break;
			}
			case SKILL:
			{
				if(getFromList(list, *index)->skill == data)
				{					
					return true;
				}
				break;
			}
		}
	}
	return false;
}

