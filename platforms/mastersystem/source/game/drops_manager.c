#include "drops_manager.h"

#include "base_defines.h"
#include "draw_utils.h"
#include "debug_utils.h"
#include "dl_rand.h"

#define DROP_FALL_SPEED 0x200
#define DROP_WIGGLE_START_TIME 0xa8 // wiggle timer starts at 168 which signed is -40. The value is decremented
								    // until it reaches 127 which then gets treated as a positive number. At that
									// point the drop falls.
#define DROP_WIGGLE_UP_SPEED 0xff80
#define DROP_WIGGLE_DOWN_SPEED 0x80
#define DROP_SPRITE_FRAME_SIZE_IN_BYTES 0xc

dl_u8 g_dropTickTockTimer;

dl_u8 drop_CollisionMasks[4] = 
{
    0xf0, // 11110000b
    0x3c, // 00111100b
    0x0f, // 00001111b
    0x03, // 00000011b
};

void DropsManager_Init(DropData* dropData, dl_u8 roomNumber, dl_u8 gameCompletionCount)
{
	int loop;
	dl_u8 dropsToInitCount = 10; // always 10 drops after finishing a cycle

	g_dropTickTockTimer = 0;

	for (loop = 0; loop < NUM_DROPS; loop++)
		dropData->drops[loop].wiggleTimer = 0;



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

	for (loop = 0; loop < dropsToInitCount; loop++)
		dropData->drops[loop].wiggleTimer = 1;
}

void wiggleDrop(Drop* drop)
{
	drop->wiggleTimer--;
	drop->speedY = (drop->wiggleTimer & 2) == 0 ? DROP_WIGGLE_UP_SPEED : DROP_WIGGLE_DOWN_SPEED;
}

void initDrop(Drop* drop, 
			  DropData* dropData, 
			  dl_u8 gameCompletionCount, 
			  const dl_u8* dropSprites,
			  dl_u8* cleanBackground)
{
	dl_u8 value;
	dl_u8 pixelMask;
	dl_u8 spriteIndex;

	// randomly pick a drop spawn area
	dl_u8 dropSpawnAreaIndex = dl_rand() % dropData->dropSpawnPositions->spawnAreasCount;
	const DropSpawnArea* dropSpawnArea = &dropData->dropSpawnPositions->dropSpawnAreas[dropSpawnAreaIndex];

	// randomly pick a position in the drop spawn area
	dl_u8 dropSpawnPointX = dl_rand() % (dropSpawnArea->dropSpawnPointsCount + 1); // spawn points count is inclusive in the original
	dropSpawnPointX *= 8; // spacing between drops. 8 pixels, not 8 bytes.


	// init drop
	drop->wiggleTimer = DROP_WIGGLE_START_TIME;


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
	value = cleanBackground[GET_FRAMEBUFFER_LOCATION(drop->x + 4, GET_HIGH_BYTE(drop->y) + 6)];
	pixelMask = pixelMasks[drop->x & 3];

	// check for a rope. If there is one, then move the
	// drop one pixel to the left, to leave clearance
	// for the player.
	if ((value & pixelMask) == pixelMask)
	{
		drop->x--;
	}

	spriteIndex = drop->x & 3; // sprite depends on which column of four pixels it lands on

	drop->spriteData = dropSprites + (DROP_SPRITE_FRAME_SIZE_IN_BYTES * spriteIndex);
	drop->collisionMask = drop_CollisionMasks[spriteIndex];

	wiggleDrop(drop);
}

void DropsManager_Update(DropData* dropData, 
						 dl_u8* framebuffer, 
						 dl_u8* cleanBackground, 
						 dl_u8 gameCompletionCount,
						 const dl_u8* dropSprites)
{
	UNUSED(framebuffer);

	int loop;
	Drop* dropsRunner = dropData->drops;

	if (g_dropTickTockTimer)
		dropsRunner++; // move to the second drop every other frame

	g_dropTickTockTimer = !g_dropTickTockTimer;

	// only process a max five drops per frame. 
	// alternating which five.
	dl_u8 count = 5;
	while (count--)
	{
		if (!dropsRunner->wiggleTimer) // we've hit an inactive drop, we've hit the end of
			return;					   // the active drops.

		if (dropsRunner->wiggleTimer == 1)
		{
			initDrop(dropsRunner, 
					 dropData, 
					 gameCompletionCount, 
					 dropSprites, 
					 cleanBackground);
		}
		else if ((dl_s8)dropsRunner->wiggleTimer < 0)
		{
			// wiggling
			wiggleDrop(dropsRunner);
		}
		else
		{
			// falling

			dl_u16 cleanBackgroundLocation = GET_FRAMEBUFFER_LOCATION(dropsRunner->x, GET_HIGH_BYTE(dropsRunner->y));

			if ((cleanBackground[cleanBackgroundLocation + 0xc0] & dropsRunner->collisionMask) || // 6 pixels down
				(cleanBackground[cleanBackgroundLocation + 0xe0] & dropsRunner->collisionMask) || // 7 pixels down
				(GET_HIGH_BYTE(dropsRunner->y) > FRAMEBUFFER_HEIGHT - 16)) // bottom of the screen bounds checking. not in the original game.
			{
#ifndef DISABLE_FRAMEBUFFER
				eraseSprite_16PixelsWide(drop->spriteData,
										 drop->x,
										 GET_HIGH_BYTE(drop->y),										  
										 DROP_SPRITE_ROWS,
										 framebuffer, 
										 cleanBackground);
#endif

				initDrop(dropsRunner, 
						 dropData, 
						 gameCompletionCount, 
						 dropSprites, 
						 cleanBackground);
			}
		}

#ifndef DISABLE_FRAMEBUFFER
		// erase drop from screen
		eraseSprite_16PixelsWide(dropsRunner->spriteData, 
								 dropsRunner->x,
								 GET_HIGH_BYTE(dropsRunner->y),
								 DROP_SPRITE_ROWS,
								 framebuffer, 
								 cleanBackground);
#endif

		// update y
		dropsRunner->y += dropsRunner->speedY;
		dropsRunner->speedY = DROP_FALL_SPEED;

#ifndef DISABLE_FRAMEBUFFER
		// draw sprite
		drawSprite_16PixelsWide(drop->spriteData, drop->x, GET_HIGH_BYTE(drop->y), DROP_SPRITE_ROWS, framebuffer);
#endif

		dropsRunner += 2; // skip to the second drop
	}
}

