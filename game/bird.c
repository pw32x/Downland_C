#include "bird.h"
#include "draw_utils.h"
#include "debug_utils.h"

#define BIRD_INACTIVE			0
#define BIRD_ACTIVE				1

#define BIRD_START_X 0x23 // 35
#define BIRD_START_Y 0x1a // 26

#define BITSHIFTED_SPRITE_FRAME_SIZE (BIRD_SPRITE_ROWS * 3) // rows * 3 bytes per row

void initBirdPhysics(BirdData* birdData)
{
	birdData->state = BIRD_ACTIVE;
	birdData->x = SET_HIGH_BYTE(BIRD_START_X);
	birdData->y = SET_HIGH_BYTE(BIRD_START_Y);
	birdData->speedx = 0x0100 + (rand() % 256);
	birdData->speedy = 0x0100 + (rand() % 256);

	birdData->currentSprite = getBitShiftedSprite(birdData->bitShiftedSprites, 
											      0,
											      BIRD_START_X & 3,
											      BITSHIFTED_SPRITE_FRAME_SIZE);
}

void Bird_Init(BirdData* birdData, u8 roomNumber, Resources* resources)
{
	birdData->state = BIRD_INACTIVE;
	birdData->bitShiftedSprites = resources->bitShiftedSprites_bird;
	birdData->animationCounter = 0;
	initBirdPhysics(birdData);
}


void Bird_Update(BirdData* birdData, u16 currentRoomTimer, u8* framebuffer, u8* cleanBackground)
{
	if (currentRoomTimer > 0)
		return;

	if (!birdData->state)
	{
		initBirdPhysics(birdData);
		return;
	}
	
	eraseSprite_24PixelsWide(framebuffer, 
							 cleanBackground,
							 GET_HIGH_BYTE(birdData->x),
							 GET_HIGH_BYTE(birdData->y),
							 birdData->currentSprite,
							 BIRD_SPRITE_ROWS);

	birdData->animationCounter++;
	birdData->animationFrame = (birdData->animationCounter >> 3) & 0x1;

	u8 newPixelY = GET_HIGH_BYTE(birdData->y + birdData->speedy);

	if (newPixelY <= 0x10 || 
		newPixelY >= 0xb1)
	{
		birdData->speedy = -birdData->speedy;
	}

	birdData->y += birdData->speedy;

	u8 newPixelX = GET_HIGH_BYTE(birdData->x + birdData->speedx);

	if (newPixelX <= 0x7 || 
		newPixelX >= 0x73)
	{
		birdData->speedx = -birdData->speedx;
	}

	birdData->x += birdData->speedx;

	birdData->currentSprite = getBitShiftedSprite(birdData->bitShiftedSprites, 
											      birdData->animationFrame,
											      GET_HIGH_BYTE(birdData->x) & 3, 
											      BITSHIFTED_SPRITE_FRAME_SIZE);

	drawSprite_24PixelsWide(birdData->currentSprite, 
							GET_HIGH_BYTE(birdData->x), 
							GET_HIGH_BYTE(birdData->y), 
							BIRD_SPRITE_ROWS, 
							framebuffer);
}
