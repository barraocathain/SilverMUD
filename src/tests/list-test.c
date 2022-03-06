#include "../misc/lists.h"
#include "../misc/playerdata.h"
#include <stdio.h>

void main()
{
	areaNode * areaList = createAreaList(createArea("Test Area A", "This is Test Area A"));
	areaNode * counter = areaList;
	addAreaNodeToList(areaList, createArea("Test Area B", "This is Test Area B"));
	addAreaNodeToList(areaList, createArea("Test Area C", "This is Test Area C"));
	for(int index = 0; index <= 2; index++)
	{
		printf("%s\n", counter->data->areaName);
		counter = counter->next;
	}
	deleteAreaNodeFromList(areaList, getAreaFromList(areaList, 1));
	addAreaNodeToList(areaList, createArea("Test Area D", "This is Test Area D"));
	counter = areaList;
	for(int index = 0; index <= 2; index++)
	{
		printf("%s\n", counter->data->areaName);
		counter = counter->next;
	}
}
