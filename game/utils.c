#include "utils.h"
#include <stdlib.h>

int dl_rand()
{
	return rand();
}

void dl_srand(u32 seed)
{
	srand(seed);
}