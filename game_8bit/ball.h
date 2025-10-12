#ifndef BALL_INCLUDE_H
#define BALL_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

#define BALL_INACTIVE			0
#define BALL_ACTIVE				1
#define BALL_DYING_MAYBE		0xff

extern dl_u8 ballData_enabled;
extern dl_u8 ballData_state;	// 0 - inactive
								// 1 - resetting?
								// 2 - active
								// 0xff - dying?
extern dl_u16 ballData_x; // high resolution position 256 pixels, 256 subpixels
extern dl_u16 ballData_y; // high resolution position 256 pixels, 256 subpixels
extern dl_u16 ballData_speedx;
extern dl_u16 ballData_speedy; // high resolution
extern dl_u8 ballData_fallStateCounter;

void Ball_Init(dl_u8 roomNumber);
void Ball_Update(void);

// it's up to the platform port to implement this function
void Ball_Draw(void);

#endif