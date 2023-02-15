#include "../src/linkedlist.h"
#include <stdio.h>

static inline void printAreaList(list * areaList)
{
	listData * currentData;
	for(int index = 0; index < areaList->itemCount; index++)
	{
		currentData = getFromList(areaList, index);
		printf("%d\t| %s - %s\n", index, currentData->area->areaName, currentData->area->areaDescription);			
	}
}

void main()
{
	list * areaList = createList(AREA);
	char areaName[256];
	char areaDescription[256];
	
	printf("\n--==[ Generating a list of ten items. ]==--\n\n");
	for(int count = 1; count <= 10; count++)
	{
		sprintf(areaName, "Area %d", count);
		sprintf(areaDescription, "This is Area %d.", count);
		
		addToList(areaList, createArea(areaName, areaDescription) , AREA);
	}
	printAreaList(areaList);

	printf("\n--==[ Inserting items into specific indexes. ]==--\n\n");
	insertIntoList(areaList, createArea("Cool, it worked.", "Cool, it worked."), AREA, 0);
	insertIntoList(areaList, createArea("Cool, it worked.", "Cool, it worked."), AREA, 6);
	insertIntoList(areaList, createArea("Cool, it worked.", "Cool, it worked."), AREA, 11);
	printAreaList(areaList);

	printf("\n--==[ Removing certain areas from the list. ]==--\n\n");
	removeFromList(areaList, AREA, 12);
	removeFromList(areaList, AREA, 6);
	removeFromList(areaList, AREA, 0);
	
	printAreaList(areaList);
	
	destroyList(&areaList);
	printf("");
}

