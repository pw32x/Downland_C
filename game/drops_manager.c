#include "drops_manager.h"

#include "base_defines.h"
#include "draw_utils.h"
#include "debug_utils.h"

#include <stdlib.h>

#define DROP_SPRITE_ROWS 6

#define DROP_FALL_SPEED 0x200
#define DROP_WIGGLE_START_TIME 0xa8 // wiggle timer starts at 168 which signed is -40. The value is decremented
								    // until it reaches 127 which then gets treated as a positive number. At that
									// point the drop falls.
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

	dropData->activeDropsCount = dropsToInitCount;

	for (int loop = 0; loop < dropsToInitCount; loop++)
		dropData->drops[loop].wiggleTimer = 1;
}

void wiggleDrop(Drop* drop)
{
	drop->wiggleTimer--;
	drop->speedY = (drop->wiggleTimer & 2) == 0 ? DROP_WIGGLE_UP_SPEED : DROP_WIGGLE_DOWN_SPEED;
}

void initDrop(Drop* drop, 
			  DropData* dropData, 
			  u8 gameCompletionCount, 
			  u8* dropSprites,
			  u8* cleanBackground)
{
	// init drop
	drop->wiggleTimer = DROP_WIGGLE_START_TIME;

	// randomly pick a drop spawn area
	u8 dropSpawnAreaIndex = rand() % dropData->dropSpawnPositions->spawnAreasCount;
	DropSpawnArea* dropSpawnArea = &dropData->dropSpawnPositions->dropSpawnAreas[dropSpawnAreaIndex];

	// randomly pick a position in the drop spawn area
	u8 dropSpawnPointX = rand() % (dropSpawnArea->dropSpawnPointsCount + 1); // spawn points count is inclusive in the original
	dropSpawnPointX *= 8; // spacing between drops. 8 pixels, not 8 bytes.

	drop->x = dropSpawnPointX + dropSpawnArea->x;
	drop->y = dropSpawnArea->y << 8;

	// if the player completed the game at least three times
	// (I'd like to see that!), adjust the drops so that they're
	// closer to the player.
	if (gameCompletionCount >= 3)
		drop->x &= 0xFE; // 11111110b to make the x position even

	// here, check if there's a collision with the background 6 pixels down and 
	// 4 to the right. If so, then move the drop's x position to the left.
	// See address 0xcfeb in the disassembly
	u8 value = cleanBackground[GET_FRAMEBUFFER_LOCATION(drop->x + 4, GET_HIGH_BYTE(drop->y) + 6)];
	u8 pixelMask = pixelMasks[drop->x & 3];

	// check for a rope. If there is one, then move the
	// drop one pixel to the left, to leave clearance
	// for the player.
	if ((value & pixelMask) == pixelMask)
	{
		drop->x--;
	}

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
			initDrop(drop, 
					 dropData, 
					 gameCompletionCount, 
					 dropSprites, 
					 cleanBackground);
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
				(GET_HIGH_BYTE(drop->y) > FRAMEBUFFER_HEIGHT - 16)) // bottom of the screen bounds checking. not in the original game.
			{
				eraseSprite_16PixelsWide(drop->spriteData,
										 drop->x,
										 GET_HIGH_BYTE(drop->y),										  
										 DROP_SPRITE_ROWS,
										 framebuffer, 
										 cleanBackground);

				initDrop(drop, 
						 dropData, 
						 gameCompletionCount, 
						 dropSprites, 
						 cleanBackground);
			}
		}

		// erase drop from screen
		eraseSprite_16PixelsWide(drop->spriteData, 
								 drop->x,
								 GET_HIGH_BYTE(drop->y),
								 DROP_SPRITE_ROWS,
								 framebuffer, 
								 cleanBackground);

		// update y
		drop->y += drop->speedY;
		drop->speedY = DROP_FALL_SPEED;

		// draw sprite
		drawSprite_16PixelsWide(drop->spriteData, drop->x, GET_HIGH_BYTE(drop->y), DROP_SPRITE_ROWS, framebuffer);

		drops += 2; // skip to the second drop
	}
}

