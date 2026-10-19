// Adapted from the Nobelia project (https://github.com/TwBurn/Nobelia)
#include <csd.h>
#include <sysio.h>
#include "input.h"

int input1Path;

int mousex;
int mousey;
int oldmousex = -1;
int oldmousey = -1;

void initInput()
{
	u_char *devName;

	devName = (u_char*)csd_devname(DT_PTR, 1);
	if (devName) 
	{
		input1Path = open(devName, UPDAT_);
		free(devName);

		if (input1Path >= 0) 
		{
			pt_org(input1Path, 0, 0);
		}
	}

	readInput();
	readInput();
}

void closeInput()
{
	if (input1Path >= 0) 
	{
		close(input1Path);
		input1Path = 0;
	}
}

// This defines a box in the middle of the screen 
// for the mouse cursor to move in. It can't go outside
// the box. The directions sent correspond to which sides 
// of the box that the mouse cursor is touching.

#define MOUSE_MOVE_X_MARGIN 64
#define MOUSE_MOVE_Y_MARGIN 32
#define MOUSE_X_RANGE_MAX 768
#define MOUSE_X_RANGE_HALF (MOUSE_X_RANGE_MAX / 2)
#define MOUSE_Y_RANGE_MAX 480
#define MOUSE_Y_RANGE_HALF (MOUSE_Y_RANGE_MAX / 2)
#define MOUSE_X_MIN	(MOUSE_X_RANGE_HALF - MOUSE_MOVE_X_MARGIN)
#define MOUSE_X_MAX (MOUSE_X_RANGE_HALF + MOUSE_MOVE_X_MARGIN)
#define MOUSE_Y_MIN (MOUSE_Y_RANGE_HALF - MOUSE_MOVE_Y_MARGIN)
#define MOUSE_Y_MAX (MOUSE_Y_RANGE_HALF + MOUSE_MOVE_Y_MARGIN)

u_short readInput() 
{
	int buttons;
	u_short inputState = 0;
	unsigned char setMousePos = 0;

	if (input1Path >= 0) 
	{
		if (oldmousex == -1 &&
			oldmousey == -1)
		{
			pt_pos(input1Path, MOUSE_X_RANGE_HALF, MOUSE_Y_RANGE_HALF);
		}

		pt_coord(input1Path, &buttons, &mousex, &mousey);	

		if (mousex < MOUSE_X_MIN)
		{
			mousex = MOUSE_X_MIN;
			setMousePos = 1;
		}

		if (mousex == MOUSE_X_MIN)
		{
			inputState |= I_LEFT;
		}

		if (mousex > MOUSE_X_MAX)
		{
			mousex = MOUSE_X_MAX;
			setMousePos = 1;
		}

		if (mousex == MOUSE_X_MAX)
		{
			inputState |= I_RIGHT;
		}

		if (mousey < MOUSE_Y_MIN)
		{
			mousey = MOUSE_Y_MIN;
			setMousePos = 1;
		}

		if (mousey == MOUSE_Y_MIN)
		{
			inputState |= I_UP;
		}

		if (mousey > MOUSE_Y_MAX)
		{
			mousey = MOUSE_Y_MAX;
			setMousePos = 1;
		}

		if (mousey == MOUSE_Y_MAX)
		{
			inputState |= I_DOWN;
		}

		if (setMousePos)
		{
			pt_pos(input1Path, mousex, mousey);
		}

		oldmousex = mousex;
		oldmousey = mousey;

		inputState |= buttons;

		return inputState;

	}
	else 
	{
		return 0xFF;
	}
}
