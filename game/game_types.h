#ifndef GAME_TYPES
#define GAME_TYPES

#include "base_types.h"
#include "base_defines.h"
#include "joystick_types.h"
#include "drops_types.h"
#include "rooms.h"
#include "resource_types.h"
#include "pickup_types.h"

#define ROOM_TIMER_DEFAULT 4096

#define ROOM_NUMBER_STRING_SIZE 2
#define TIMER_STRING_SIZE 6
#define SCORE_STRING_SIZE 8

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
	u8 numPlayers;
	u8 currentPlayer;
	u8 playerLives;

	RoomPickups gamePickups;

	// used for screen transitions
	u8 transitionRoomNumber;
	u16 transitionInitialDelay;
	u8 transitionCurrentLine;
	u8 transitionFrameDelay;

	// strings. Number of expected characters + 0xff ending value
	u8 string_roomNumber[ROOM_NUMBER_STRING_SIZE];
	u8 string_timer[TIMER_STRING_SIZE]; // max timer is 65535
	u8 string_score[SCORE_STRING_SIZE];

	// timers
	u16 roomTimers[NUM_ROOMS];

	u32 score;
} GameData;

#endif