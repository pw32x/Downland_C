#include "drops_manager.h"

#include "base_defines.h"
#include "draw_utils.h"

//#include <stdlib.h>

#define DROP_SPRITE_ROWS 6

#define DROP_FALL_SPEED 0x200
#define DROP_WIGGLE_UP_SPEED 0xff80
#define DROP_WIGGLE_DOWN_SPEED 0x80
#define DROP_SPRITE_FRAME_SIZE_IN_BYTES 0xc

u8 g_dropTickTockTimer;

u8 drop_CollisionMasks[4] = 
{
    0xf0, // 11110000b
    0x3c, // 00111100b
    0x0f, // 00001111b
    0x03, // 00000011b
};

void DropsManager_Init(DropData* dropData, u8 roomNumber, u8 gameCompletionCount)
{
	srand(34980);

	g_dropTickTockTimer = 0;

	for (int loop = 0; loop < NUM_DROPS; loop++)
		dropData->drops[loop].wiggleTimer = 0;

	u8 dropsToInitCount = 10; // always 10 drops after finishing a cycle

	if (!gameCompletionCount)
	{
		// the number of drops is always room_number + 1,
		// except the first four rooms, where it's always 6

		if (roomNumber < 5)
			dropsToInitCount = 5;
		else 
			dropsToInitCount = roomNumber;

		dropsToInitCount++; // always one more
	}

	for (int loop = 0; loop < dropsToInitCount; loop++)
		dropData->drops[loop].wiggleTimer = 1;
}

void wiggleDrop(Drop* drop)
{
	drop->wiggleTimer--;
	drop->speedY = (drop->wiggleTimer & 2) == 0 ? DROP_WIGGLE_UP_SPEED : DROP_WIGGLE_DOWN_SPEED;
}

void initDrop(Drop* drop, DropData* dropData, u8 gameCompletionCount, u8* dropSprites)
{
	// init drop
	drop->wiggleTimer = 0xa8;

	// randomly pick a drop spawn area
	u8 dropSpawnAreaIndex = rand() % dropData->dropSpawnPositions->spawnAreasCount;
	DropSpawnArea* dropSpawnArea = &dropData->dropSpawnPositions->dropSpawnAreas[dropSpawnAreaIndex];

	// randomly pick a position in the drop spawn area
	u8 dropSpawnPointX = rand() % (dropSpawnArea->dropSpawnPointsCount + 1); // spawn points count is inclusive in the original
	dropSpawnPointX *= 8; // spacing between drops. 8 pixels, not 8 bytes.

	drop->x = dropSpawnPointX + dropSpawnArea->x;
	drop->y = dropSpawnArea->y << 8;

	if (gameCompletionCount < 3)
		drop->x &= 0xFE; // 11111110b to ensure even positions for x? 

	// TODO
	// here, check if there's a collision with the background 6 pixels down and 
	// 4 to the right. If so, then move the drop's x position to the left.
	// See address 0xcfeb in the disassembly

	u8 spriteIndex = drop->x & 3; // sprite depends on which column of four pixels it lands on

	drop->spriteData = dropSprites + (DROP_SPRITE_FRAME_SIZE_IN_BYTES * spriteIndex);
	drop->collisionMask = drop_CollisionMasks[spriteIndex];

	wiggleDrop(drop);
}

void DropsManager_Update(DropData* dropData, 
						 u8* framebuffer, 
						 u8* cleanBackground, 
						 u8 gameCompletionCount,
						 u8* dropSprites)
{
	Drop* drops = dropData->drops;

	if (g_dropTickTockTimer)
		drops++; // move to the second drop every other frame

	g_dropTickTockTimer = !g_dropTickTockTimer;

	// only process a max five drops per frame. 
	// alternating which five.
	for (int loop = 0; loop < 5; loop++) 
	{
		Drop* drop = drops;
		if (!drop->wiggleTimer) // we've hit an inactive drop, we've hit the end of
			return;				// the active drops.

		if (drop->wiggleTimer == 1)
		{
			initDrop(drop, dropData, gameCompletionCount, dropSprites);
		}
		else if ((s8)drop->wiggleTimer < 0)
		{
			// wiggling
			wiggleDrop(drop);
		}
		else
		{
			// falling

			u16 cleanBackgroundLocation = GET_FRAMEBUFFER_LOCATION(drop->x, GET_HIGH_BYTE(drop->y));

			if ((cleanBackground[cleanBackgroundLocation + 0xc0] & drop->collisionMask) || // 6 pixels down
				(cleanBackground[cleanBackgroundLocation + 0xe0] & drop->collisionMask) || // 7 pixels down
				(GET_HIGH_BYTE(drop->y) > FRAMEBUFFER_HEIGHT - 16)) // bottom bounds checking. not in the original game.
			{
				eraseSprite_16PixelsWide(framebuffer, 
										 cleanBackground, 
										 drop->x,
										 GET_HIGH_BYTE(drop->y),
										 drop->spriteData, 
										 DROP_SPRITE_ROWS);

				initDrop(drop, dropData, gameCompletionCount, dropSprites);
			}
		}

		// erase drop from screen
		eraseSprite_16PixelsWide(framebuffer, 
								 cleanBackground, 
								 drop->x,
								 GET_HIGH_BYTE(drop->y),
								 drop->spriteData, 
								 DROP_SPRITE_ROWS);

		// update y
		drop->y += drop->speedY;
		drop->speedY = DROP_FALL_SPEED;

		// draw sprite
		drawSprite_16PixelsWide(drop->spriteData, drop->x, GET_HIGH_BYTE(drop->y), DROP_SPRITE_ROWS, framebuffer);

		drops += 2; // skip to the second drop
	}
}