#ifndef GAME_TYPES
#define GAME_TYPES

#include "base_types.h"
#include "base_defines.h"
#include "joystick_types.h"
#include "drops_types.h"
#include "rooms/rooms.h"
#include "resource_types.h"
#include "pickup_types.h"
#include "string_utils.h"
#include "ball.h"
#include "bird.h"
#include "player.h"

#define NUM_PLAYERS 2
#define PLAYER_ONE	0
#define PLAYER_TWO	1

// contains the global state of the game
typedef struct GameData
{
	dl_u8* cleanBackground; // the game background without UI or objects. Used for terrain collision detection.

	// objects
	PlayerData playerData[NUM_PLAYERS];
	DropData dropData;
	BallData ballData;
	BirdData birdData;

	JoystickState joystickState;

	// there's no distinct player one or player two
	// the pointers swap every time a player dies.
	PlayerData* currentPlayerData;
	PlayerData* otherPlayerData;
	dl_u8 numPlayers;

	Room* currentRoom;

	// used for screen transitions
	dl_u8 transitionRoomNumber;
	dl_u16 transitionInitialDelay;
	dl_u8 transitionCurrentLine;
	dl_u8 transitionFrameDelay;

	// strings. Number of expected characters + 0xff ending value
	dl_u8 string_roomNumber[ROOM_NUMBER_STRING_SIZE];
	dl_u8 string_timer[TIMER_STRING_SIZE]; // max timer is 65535
	dl_u8 string_highScore[SCORE_STRING_SIZE];

	dl_u32 highScore;
	dl_u8 paused;
} GameData;

#endif