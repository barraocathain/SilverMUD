// schemeintegration.h: Data-structures and function prototypes for SilverMUD's Scheme integration.
// Barra Ó Catháin, 2023.
#ifndef SCHEMEINTEGRATION_H
#define SCHEMEINTEGRATION_H
#include "queue.h"
#include "linkedlist.h"

typedef struct list list;
typedef struct queue queue;

typedef struct SchemeThreadParameters
{
	list * areaList, * skillList;
	queue * inputQueue, * outputQueue;
} SchemeThreadParameters;

void * schemeHandler(void * parameters);

#endif
