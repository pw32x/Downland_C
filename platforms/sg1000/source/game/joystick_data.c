#include "joystick_data.h"

dl_u8 joystickState_leftDown;
dl_u8 joystickState_rightDown;
dl_u8 joystickState_upDown;
dl_u8 joystickState_downDown;
dl_u8 joystickState_jumpDown;

dl_u8 joystickState_leftPressed;
dl_u8 joystickState_rightPressed;
dl_u8 joystickState_jumpPressed;

#ifdef DEV_MODE
dl_u8 joystickState_debugStateDown;
dl_u8 joystickState_debugStatePressed;
#endif