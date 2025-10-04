#ifndef BIRD_INCLUDE_H
#define BIRD_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

// It's a bird! It's a bat! It's a flying tomato!

#define BIRD_INACTIVE			0
#define BIRD_ACTIVE				1
#define BIRD_SHUTDOWN			2

extern dl_u8 birdData_state;	// 0 - inactive
								// 1 - active
extern dl_u16 birdData_x; // high resolution position 256 pixels, 256 subpixels
extern dl_u16 birdData_y; // high resolution position 256 pixels, 256 subpixels
extern dl_u16 birdData_speedx;
extern dl_u16 birdData_speedy; // high resolution
extern dl_u8 birdData_animationCounter;
extern dl_u8 birdData_animationFrame;

void Bird_Init(void);
void Bird_Update(dl_u16 currentRoomTimer);
void Bird_Draw(dl_u16 currentTimer);

#endif