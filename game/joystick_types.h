#ifndef JOYSTICK_TYPES_INCLUDE_H
#define JOYSTICK_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"

typedef struct
{
	dl_u8 leftDown;
	dl_u8 rightDown;
	dl_u8 upDown;
	dl_u8 downDown;
	dl_u8 jumpDown;
	dl_u8 startDown;

	dl_u8 leftPressed;
	dl_u8 rightPressed;
	dl_u8 upPressed;
	dl_u8 downPressed;
	dl_u8 jumpPressed;
	dl_u8 startPressed;

	dl_u8 leftReleased;
	dl_u8 rightReleased;
	dl_u8 upReleased;
	dl_u8 downReleased;
	dl_u8 jumpReleased;
	dl_u8 startReleased;

#ifdef DEV_MODE
	dl_u8 debugStateDown;
	dl_u8 debugStatePressed;
	dl_u8 debugStateReleased;
#endif

} JoystickState;

#endif