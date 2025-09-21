#include "bird.h"
#include "dl_rand.h"
#include "dl_platform.h"

dl_u8 birdData_state;	// 0 - inactive
						// 1 - active
dl_u16 birdData_x; // high resolution position 256 pixels, 256 subpixels
dl_u16 birdData_y; // high resolution position 256 pixels, 256 subpixels
dl_u16 birdData_speedx;
dl_u16 birdData_speedy; // high resolution
dl_u8 birdData_animationCounter;
dl_u8 birdData_animationFrame;

#define BIRD_START_X 0x23 // 35
#define BIRD_START_Y 0x1a // 26

#define SCREEN_BOUNDS_LEFT		0x7
#define SCREEN_BOUNDS_RIGHT		0x73
#define SCREEN_BOUNDS_TOP		0x10
#define SCREEN_BOUNDS_BOTTOM	0xb1

#define BITSHIFTED_SPRITE_FRAME_SIZE (BIRD_SPRITE_ROWS * 3) // rows * 3 bytes per row

void initBirdPhysics(void)
{
	birdData_state = BIRD_ACTIVE;
	birdData_x = SET_HIGH_BYTE(BIRD_START_X);
	birdData_y = SET_HIGH_BYTE(BIRD_START_Y);
	birdData_speedx = 0x0100 + (dl_rand() % 256);
	birdData_speedy = 0x0100 + (dl_rand() % 256);
}

void Bird_Init(void)
{
	birdData_state = BIRD_INACTIVE;
	birdData_animationCounter = 0;
}


void Bird_Update(dl_u16 currentRoomTimer)
{
	dl_u8 newPixelX;
	dl_u8 newPixelY;

	if (birdData_state == BIRD_SHUTDOWN)
	{
		birdData_state = BIRD_INACTIVE;
	}

	if (currentRoomTimer > 0)
		return;

	if (!birdData_state)
	{
		initBirdPhysics();
		return;
	}

	birdData_animationCounter++;
	birdData_animationFrame = (birdData_animationCounter >> 3) & 0x1;

	newPixelY = GET_HIGH_BYTE(birdData_y + birdData_speedy);

	if (newPixelY <= SCREEN_BOUNDS_TOP || 
		newPixelY >= SCREEN_BOUNDS_BOTTOM)
	{
		birdData_speedy = -birdData_speedy;
	}

	birdData_y += birdData_speedy;

	newPixelX = GET_HIGH_BYTE(birdData_x + birdData_speedx);

	if (newPixelX <= SCREEN_BOUNDS_LEFT || 
		newPixelX >= SCREEN_BOUNDS_RIGHT)
	{
		birdData_speedx = -birdData_speedx;
	}

	birdData_x += birdData_speedx;
}

#define BIRD_TILE_INDEX 8

void Bird_Draw(dl_u16 currentTimer)
{
	// draw bird
	if (birdData_state && currentTimer == 0)
	{
		SMS_addTwoAdjoiningSprites((birdData_x >> 8) << 1,
									birdData_y >> 8,
									BIRD_TILE_INDEX + (birdData_animationFrame << 2));
	}
}