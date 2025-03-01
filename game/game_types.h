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

#define ROOM_TIMER_DEFAULT 4096

typedef struct
{
	u8 framebuffer[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // main game 1bpp frame buffer
	u8 cleanBackground[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // the game background without UI or objects. Used for terrain collision detection.
	u32 crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects
	DropData dropData;
	u8 gameCompletionCount;
	JoystickState joystickState;
	Room* currentRoom;
	u8 numPlayers;
	u8 currentPlayer;
	u8 playerLives;

	RoomPickups gamePickups;
	u8 doorStateData[DOOR_TOTAL_COUNT];

	BallData ballData;
	BirdData birdData;
	PlayerData playerData;

	// used for screen transitions
	u8 transitionRoomNumber;
	u16 transitionInitialDelay;
	u8 transitionCurrentLine;
	u8 transitionFrameDelay;

	// strings. Number of expected characters + 0xff ending value
	u8 string_roomNumber[ROOM_NUMBER_STRING_SIZE];
	u8 string_timer[TIMER_STRING_SIZE]; // max timer is 65535
	u8 string_playerOneScore[SCORE_STRING_SIZE];
	u8 string_playerTwoScore[SCORE_STRING_SIZE];
	u8 string_highScore[SCORE_STRING_SIZE];

	// timers
	u16 roomTimers[NUM_ROOMS];

	u32 playerOneScore;
	u32 playerTwoScore;
	u32 highScore;

	u32 paused;
} GameData;

#endif