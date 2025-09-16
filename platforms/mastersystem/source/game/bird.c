#include "bird.h"
#include "draw_utils.h"
#include "dl_rand.h"

#define BIRD_START_X 0x23 // 35
#define BIRD_START_Y 0x1a // 26

#define SCREEN_BOUNDS_LEFT		0x7
#define SCREEN_BOUNDS_RIGHT		0x73
#define SCREEN_BOUNDS_TOP		0x10
#define SCREEN_BOUNDS_BOTTOM	0xb1

#define BITSHIFTED_SPRITE_FRAME_SIZE (BIRD_SPRITE_ROWS * 3) // rows * 3 bytes per row



void initBirdPhysics(BirdData* birdData)
{
	birdData->state = BIRD_ACTIVE;
	birdData->x = SET_HIGH_BYTE(BIRD_START_X);
	birdData->y = SET_HIGH_BYTE(BIRD_START_Y);
	birdData->speedx = 0x0100 + (dl_rand() % 256);
	birdData->speedy = 0x0100 + (dl_rand() % 256);
}

void Bird_Init(BirdData* birdData)
{
	birdData->state = BIRD_INACTIVE;
	birdData->animationCounter = 0;
}


void Bird_Update(BirdData* birdData, dl_u16 currentRoomTimer)
{
	dl_u8 newPixelX;
	dl_u8 newPixelY;

	if (birdData->state == BIRD_SHUTDOWN)
	{
		birdData->state = BIRD_INACTIVE;
	}

	if (currentRoomTimer > 0)
		return;

	if (!birdData->state)
	{
		initBirdPhysics(birdData);
		return;
	}

	birdData->animationCounter++;
	birdData->animationFrame = (birdData->animationCounter >> 3) & 0x1;

	newPixelY = GET_HIGH_BYTE(birdData->y + birdData->speedy);

	if (newPixelY <= SCREEN_BOUNDS_TOP || 
		newPixelY >= SCREEN_BOUNDS_BOTTOM)
	{
		birdData->speedy = -birdData->speedy;
	}

	birdData->y += birdData->speedy;

	newPixelX = GET_HIGH_BYTE(birdData->x + birdData->speedx);

	if (newPixelX <= SCREEN_BOUNDS_LEFT || 
		newPixelX >= SCREEN_BOUNDS_RIGHT)
	{
		birdData->speedx = -birdData->speedx;
	}

	birdData->x += birdData->speedx;
}
