#ifndef JOYSTICK_TYPES_INCLUDE_H
#define JOYSTICK_TYPES_INCLUDE_H

typedef struct
{
	u8 leftDown;
	u8 rightDown;
	u8 upDown;
	u8 downDown;
	u8 jumpDown;

	u8 leftPressed;
	u8 rightPressed;
	u8 upPressed;
	u8 downPressed;
	u8 jumpPressed;

	u8 leftReleased;
	u8 rightReleased;
	u8 upReleased;
	u8 downReleased;
	u8 jumpReleased;

} JoystickState;

#endif