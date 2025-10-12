#ifndef JOYSTICK_DATA_INCLUDE_H
#define JOYSTICK_DATA_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"


extern dl_u8 joystickState_leftDown;
extern dl_u8 joystickState_rightDown;
extern dl_u8 joystickState_upDown;
extern dl_u8 joystickState_downDown;
extern dl_u8 joystickState_jumpDown;
extern dl_u8 joystickState_startDown;

extern dl_u8 joystickState_leftPressed;
extern dl_u8 joystickState_rightPressed;
extern dl_u8 joystickState_jumpPressed;
extern dl_u8 joystickState_startPressed;

#ifdef DEV_MODE
extern dl_u8 joystickState_debugStateDown;
extern dl_u8 joystickState_debugStatePressed;
#endif

#endif