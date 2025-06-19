#include "..\..\game\alloc.h"

static u8 memory[18288];
static u8* memoryEnd = NULL;

void* dl_alloc(u32 size)
{
	if (memoryEnd == NULL)
	{
		memoryEnd = memory;
	}

	u8* memory = memoryEnd;

	memoryEnd += size;

	return (void*)memory;
}