#include "erase_system.h"

typedef struct
{
	dl_u16 x;
	dl_u16 y;
	dl_u8 height;
	EraseFunc eraseFunc;
} EraseEntry;

#define NUM_OBJECTS 48
EraseEntry g_eraseList0[NUM_OBJECTS];
EraseEntry g_eraseList1[NUM_OBJECTS];

dl_u8 g_eraseList0Count;
dl_u8 g_eraseList1Count;

EraseEntry* g_currentEraseList;
dl_u8* g_currentEraseListCount;

EraseEntry* g_otherEraseList;
dl_u8* g_otherEraseListCount;

void initEraseList()
{
	g_eraseList0Count = 0;
	g_eraseList1Count = 0;

	g_currentEraseList = g_eraseList0;
	g_currentEraseListCount = &g_eraseList0Count;

	g_otherEraseList = g_eraseList1;
	g_otherEraseListCount = &g_eraseList1Count;
}

void addToEraseList(dl_u16 x, dl_u16 y, dl_u8 height, EraseFunc eraseFunc)
{
	EraseEntry* currentEntry = &g_currentEraseList[(*g_currentEraseListCount)];
	currentEntry->x = x;
	currentEntry->y = y;
	currentEntry->height = height;
	currentEntry->eraseFunc = eraseFunc;

	(*g_currentEraseListCount)++;
}

void addToOtherEraseList(dl_u16 x, dl_u16 y, dl_u8 height, EraseFunc eraseFunc)
{
	EraseEntry* otherEntry = &g_otherEraseList[(*g_otherEraseListCount)];
	otherEntry->x = x;
	otherEntry->y = y;
	otherEntry->height = height;
	otherEntry->eraseFunc = eraseFunc;

	(*g_otherEraseListCount)++;
}


void processEraseList()
{
	dl_u8 loop;
	EraseEntry* eraseListRunner;

	if (g_currentEraseList == g_eraseList0)
	{
		g_currentEraseList = g_eraseList1;
		g_currentEraseListCount = &g_eraseList1Count;

		g_otherEraseList = g_eraseList0;
		g_otherEraseListCount = &g_eraseList0Count;
	}
	else
	{
		g_currentEraseList = g_eraseList0;
		g_currentEraseListCount = &g_eraseList0Count;

		g_otherEraseList = g_eraseList1;
		g_otherEraseListCount = &g_eraseList1Count;
	}

	eraseListRunner = g_currentEraseList;
	for (loop = 0; loop < (*g_currentEraseListCount); loop++)
	{
		eraseListRunner->eraseFunc(eraseListRunner->x, eraseListRunner->y, eraseListRunner->height);
		eraseListRunner++;
	}

	(*g_currentEraseListCount) = 0;
}
