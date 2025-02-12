#ifndef GAME_TYPES
#define GAME_TYPES

#include "base_types.h"
#include "base_defines.h"
#include "joystick_types.h"
#include "drops_types.h"
#include "rooms.h"
#include "resource_types.h"


typedef struct
{
	u8 framebuffer[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // main game 1bpp frame buffer
	u8 cleanBackground[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // the game background without UI or objects. Used for terrain collision detection.
	u32 crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects
	DropData dropData;
	u8 gameCompletionCount;
	JoystickState joystickState;
	Resources* resources;
	Room* currentRoom;
} GameData;

#endif