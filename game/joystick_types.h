#ifndef JOYSTICK_TYPES_INCLUDE_H
#define JOYSTICK_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"

typedef struct
{
	u8 leftDown;
	u8 rightDown;
	u8 upDown;
	u8 downDown;
	u8 jumpDown;
	u8 startDown;

	u8 leftPressed;
	u8 rightPressed;
	u8 upPressed;
	u8 downPressed;
	u8 jumpPressed;
	u8 startDownPressed;

	u8 leftReleased;
	u8 rightReleased;
	u8 upReleased;
	u8 downReleased;
	u8 jumpReleased;
	u8 startDownReleased;

#ifdef DEV_MODE
	u8 debugStateDown;
	u8 debugStatePressed;
	u8 debugStateReleased;
#endif

} JoystickState;

#endif