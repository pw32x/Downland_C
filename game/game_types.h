#ifndef GAME_TYPES
#define GAME_TYPES

#include "base_types.h"
#include "base_defines.h"
#include "joystick_types.h"
#include "drops_types.h"
#include "rooms.h"
#include "resource_types.h"
#include "pickup_types.h"
#include "string_utils.h"
#include "ball.h"
#include "bird.h"
#include "player.h"

#define NUM_PLAYERS 2
#define PLAYER_ONE	0
#define PLAYER_TWO	1

typedef struct GameData
{
	u8 framebuffer[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // main game 1bpp frame buffer
	u8 cleanBackground[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // the game background without UI or objects. Used for terrain collision detection.
	u32 crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects

	DropData dropData;
	BallData ballData;
	BirdData birdData;

	JoystickState joystickState;

	u8 numPlayers;

	PlayerData playerData[NUM_PLAYERS];
	PlayerData* currentPlayerData;
	PlayerData* otherPlayerData;

	Room* currentRoom;

	// used for screen transitions
	u8 transitionRoomNumber;
	u16 transitionInitialDelay;
	u8 transitionCurrentLine;
	u8 transitionFrameDelay;

	// strings. Number of expected characters + 0xff ending value
	u8 string_roomNumber[ROOM_NUMBER_STRING_SIZE];
	u8 string_timer[TIMER_STRING_SIZE]; // max timer is 65535
	u8 string_highScore[SCORE_STRING_SIZE];

	u32 highScore;
	u8 paused;
} GameData;

#endif