#include "drops_manager.h"

#include <stdlib.h>

#include "base_defines.h"

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

void eraseSprite(u8* framebuffer, 
				 u8* cleanBackground,
				 u16 framebufferDrawLocation, 
				 u8* spriteData, 
				 u8 rowCount)
{
	framebuffer += framebufferDrawLocation;
	cleanBackground += framebufferDrawLocation;

	for (int loop = 0; loop < rowCount; loop++)
	{
		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;
		framebuffer++;
		cleanBackground++;

		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;

		framebuffer += (FRAMEBUFFER_PITCH - 1); // move to the next row
		cleanBackground += (FRAMEBUFFER_PITCH - 1);
	}
}

void drawSprite(u8* framebuffer, 
				u16 framebufferDrawLocation, 
				u8* spriteData, 
				u8 rowCount)
{
	framebuffer += framebufferDrawLocation;

	for (int loop = 0; loop < rowCount; loop++)
	{
		// blend the sprite's pixels with the frame buffer
		*framebuffer |= *spriteData;
		spriteData++;
		framebuffer++;

		// blend the sprite's pixels with the frame buffer
		*framebuffer |= *spriteData;
		spriteData++;

		framebuffer += (FRAMEBUFFER_PITCH - 1); // move to the next row
	}
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

	drop->framebufferDrawLocation = (drop->x / 4) + (GET_FROM_HIRES(drop->y) * FRAMEBUFFER_PITCH);
	drop->previousFramebufferDrawLocation = drop->framebufferDrawLocation;

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

			if ((cleanBackground[drop->previousFramebufferDrawLocation + 0xc0] & drop->collisionMask) || // 6 pixels down
				(cleanBackground[drop->previousFramebufferDrawLocation + 0xe0] & drop->collisionMask) || // 7 pixels down
				(GET_FROM_HIRES(drop->y) > FRAMEBUFFER_HEIGHT - 16)) // bottom bounds checking. not in the original game.
			{
				eraseSprite(framebuffer, cleanBackground, drop->previousFramebufferDrawLocation, drop->spriteData, 6);
				initDrop(drop, dropData, gameCompletionCount, dropSprites);
			}
		}

		// update y
		drop->y += drop->speedY;
		drop->speedY = DROP_FALL_SPEED;

		// compute framebuffer location
		drop->framebufferDrawLocation = (drop->x / 4) + (GET_FROM_HIRES(drop->y) * FRAMEBUFFER_PITCH);

		// erase drop from screen
		eraseSprite(framebuffer, cleanBackground, drop->previousFramebufferDrawLocation, drop->spriteData, 6);

		// store framebuffer location to previous framebuffer location
		drop->previousFramebufferDrawLocation = drop->framebufferDrawLocation;

		// draw sprite
		drawSprite(framebuffer, drop->framebufferDrawLocation, drop->spriteData, 6);

		drops += 2; // skip to the second drop
	}
}