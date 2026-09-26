#include <csd.h>
#include <sysio.h>
#include "input.h"

#define PT_CENTER 256
#define DEADZONE 1

int input1Path, input2Path;
u_short input1State, input2State;

int dx1, dx2;
int dy1, dy2;

void initInput()
{
	u_char *devName;

	devName = (u_char*)csd_devname(DT_PTR, 1);
	if (devName) {
		input1Path = open(devName, UPDAT_);
		free(devName);

		if (input1Path >= 0) {
			pt_org(input1Path, 0, 0);
		}
	}

	devName = (u_char*)csd_devname(DT_PTR, 2);
	if (devName) {
		input2Path = open(devName, UPDAT_);
		free(devName);

		if (input2Path >= 0) {
			pt_org(input2Path, 0, 0);
		}
	}

	readInput1();
	readInput1();
	readInput2();
	readInput2();
}

void closeInput()
{
	if (input1Path >= 0) {
		close(input1Path);
		input1Path = 0;
	}
	if (input2Path >= 0) {
		close(input2Path);
		input2Path = 0;
	}
}



u_short readInput1() {
	int x, y, buttons;
	int dx, dy;
	u_short inputState = 0;

	if (input1Path >= 0) {
		pt_coord(input1Path, &buttons, &x, &y);
		buttons &= 3;
		if (buttons == 3) {
			inputState |= I_BUTTON3;
		}
		else {
			inputState |= buttons;
		}
		
		x = x - PT_CENTER;
		y = y - PT_CENTER;

		dx = dx1 + x;
		dy = dy1 + y;

		/* Handle coordinate wrap-around */
				
		if (dx < -DEADZONE) inputState |= I_LEFT;
		if (dx >  DEADZONE) inputState |= I_RIGHT;
		if (dy < -DEADZONE) inputState |= I_UP;
		if (dy >  DEADZONE) inputState |= I_DOWN;

		dx1 = x;
		dy1 = y;

		if (x || y) pt_pos(input1Path, PT_CENTER, PT_CENTER);

		return inputState;
	}
	else {
		return 0xFF;
	}
}

u_short readInput2() {
	int x, y, buttons;
	int dx, dy;
	u_short inputState = 0;

	if (input2Path >= 0) {
		pt_coord(input2Path, &buttons, &x, &y);
		buttons &= 3;
		if (buttons == 3) {
			inputState |= I_BUTTON3;
		}
		else {
			inputState |= buttons;
		}
		
		x = x - PT_CENTER; /* Discard last bits to ignore very small movement */
		y = y - PT_CENTER;

		dx = dx2 + x;
		dy = dy2 + y;

		/* Handle coordinate wrap-around */
				
		if (dx < -DEADZONE) inputState |= I_LEFT;
		if (dx >  DEADZONE) inputState |= I_RIGHT;
		if (dy < -DEADZONE) inputState |= I_UP;
		if (dy >  DEADZONE) inputState |= I_DOWN;

		dx2 = x;
		dy2 = y;

		if (x || y) pt_pos(input2Path, PT_CENTER, PT_CENTER);

		return inputState;
	}
	else {
		return 0xFF;
	}
}
