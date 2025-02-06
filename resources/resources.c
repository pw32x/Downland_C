#include "resources.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Checks for Downland V1.1
BOOL checksumCheck(const char* romPath)
{
	u32 accumulator = 0;
	u32 value;

	FILE* file = fopen(romPath, "rb");

	while (fread(&value, sizeof(value), 1, file))
	{
		accumulator += value;
	}

	fclose(file);

	return accumulator == 0x84883253;
}

byte* getBytes(FILE* file, u16 start, u16 end)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 
	end -= 0xc000;

	u16 size = end - start;

	byte* memory = (byte*)malloc(size);

	if (memory == NULL)
			return NULL;

	fseek(file, start, SEEK_SET);
	fread(memory, size, 1, file);

	return memory;
}

BOOL Resources_Init(const char* romPath, Resources* resources)
{
	if (!checksumCheck(romPath))
		return FALSE;

	FILE* file = fopen(romPath, "rb");

	if (file == NULL)
		return FALSE;

	// get character font
	resources->characterFont = getBytes(file, 0xd908, 0xda19);

	// get strings
	resources->text_downland = getBytes(file, 0xda19, 0xda27);
	resources->text_writtenBy = getBytes(file, 0xda27, 0xda33);
	resources->text_michaelAichlmayer = getBytes(file, 0xda33, 0xda45);
	resources->text_copyright1983 = getBytes(file, 0xda45, 0xda54);
	resources->text_spectralAssociates = getBytes(file, 0xda54, 0xda68);
	resources->text_licensedTo = getBytes(file, 0xda68, 0xda75);
	resources->text_tandyCorporation = getBytes(file, 0xda75, 0xda87);
	resources->text_allRightsReserved = getBytes(file, 0xda87, 0xda9b);
	resources->text_onePlayer = getBytes(file, 0xda9b, 0xdaa6);
	resources->text_twoPlayer = getBytes(file, 0xdaa6, 0xdab1);
	resources->text_highScore = getBytes(file, 0xdab1, 0xdabc);
	resources->text_playerOne = getBytes(file, 0xdabc, 0xdac7);
	resources->text_playerTwo = getBytes(file, 0xdac7, 0xdad2);
	resources->text_pL1 = getBytes(file, 0xdad2, 0xdad6);
	resources->text_pL2 = getBytes(file, 0xdad6, 0xdada);
	resources->text_getReadyPlayerOne = getBytes(file, 0xdada, 0xdaef);
	resources->text_getReadyPlayerTwo = getBytes(file, 0xdaef, 0xdb04);
	resources->text_chamber = getBytes(file, 0xdb04, 0xdb0c);

	// get sprites
    resources->sprites_player = getBytes(file, 0xdcd6, 0xde17);
    resources->collisionmask_player = getBytes(file, 0xde17, 0xde7b);
    resources->sprites_bouncyBall = getBytes(file, 0xde7b, 0xde9b);
    resources->sprites_bird = getBytes(file, 0xde9b, 0xdeb3);
    resources->sprite_moneyBag = getBytes(file, 0xdeb3, 0xdec7);
    resources->sprite_diamond = getBytes(file, 0xdec7, 0xd3db);
    resources->sprite_key = getBytes(file, 0xd3db, 0xdeef);
    resources->sprite_playerSplat = getBytes(file, 0xdeef, 0xdf0a);
    resources->sprite_door = getBytes(file, 0xdf0a, 0xdf2a);
    resources->sprites_drops = getBytes(file, 0xdf2a, 0xdf5a);

	fclose(file);

	return TRUE;
}

void Resources_Shutdown(Resources* resources)
{
	free(resources->characterFont);
}