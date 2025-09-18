#include "game.h"

#include "dl_platform.h"
#include "rooms/rooms.h"
#include "game_data.h"
#include "joystick_data.h"

Game_ChangedRoomCallbackType Game_ChangedRoomCallback = NULL;
Game_ChangedRoomCallbackType Game_TransitionDone = NULL;

void Game_Init(void)
{
	dl_memset((void*)&gameData_playerData, 0, sizeof(gameData_playerData));
	
	joystickState_leftDown = FALSE;
	joystickState_rightDown = FALSE;
	joystickState_upDown = FALSE;
	joystickState_downDown = FALSE;
	joystickState_jumpDown = FALSE;
	joystickState_leftPressed = FALSE;
	joystickState_rightPressed = FALSE;
	joystickState_jumpPressed = FALSE;

	gameData_paused = 0;
	gameData_numPlayers = 1;

	// init strings
	gameData_string_roomNumber[ROOM_NUMBER_STRING_SIZE - 1] = 0xff; // end of line
	gameData_string_timer[TIMER_STRING_SIZE - 1] = 0xff;
	gameData_playerData[PLAYER_ONE].scoreString[SCORE_STRING_SIZE - 1] = 0xff;
	gameData_playerData[PLAYER_TWO].scoreString[SCORE_STRING_SIZE - 1] = 0xff;
	gameData_string_highScore[SCORE_STRING_SIZE - 1] = 0xff;

	gameData_highScore = 0;

	gameData_playerData[PLAYER_ONE].playerNumber = PLAYER_ONE;
	gameData_playerData[PLAYER_ONE].playerMask = PLAYERONE_BITMASK;

	gameData_playerData[PLAYER_TWO].playerNumber = PLAYER_TWO;
	gameData_playerData[PLAYER_TWO].playerMask = PLAYERTWO_BITMASK;

#ifdef START_AT_TITLESCREEN
	// init title screen
	Game_TransitionToRoom(TITLESCREEN_ROOM_INDEX);
#else
	gameData_currentPlayerData = &gameData_playerData[PLAYER_ONE];
	gameData_otherPlayerData = NULL;
	Player_GameInit(gameData_currentPlayerData);
	Game_TransitionToRoom(gameData, 0 /*roomNumber*/);
#endif
}

void Game_InitPlayers(void)
{
	dl_u8 loop;

	gameData_currentPlayerData = &gameData_playerData[PLAYER_ONE];

	gameData_otherPlayerData = gameData_numPlayers > 1 ? &gameData_playerData[PLAYER_TWO] : NULL;

	for (loop = 0; loop < gameData_numPlayers; loop++)
	{
		Player_GameInit(&gameData_playerData[loop]);
	}
}

void Game_Update(void)
{
	gameData_currentRoom->update((struct Room*)gameData_currentRoom);
}

void Game_EnterRoom(dl_u8 roomNumber)
{
	gameData_currentRoom = g_rooms[roomNumber];

	if (gameData_currentPlayerData != NULL)
		gameData_currentPlayerData->currentRoom = gameData_currentRoom;

	gameData_currentRoom->init(gameData_currentRoom);

	if (Game_ChangedRoomCallback != NULL)
		Game_ChangedRoomCallback(roomNumber, -1);
}

void Game_TransitionToRoom(dl_u8 roomNumber)
{
	gameData_transitionRoomNumber = roomNumber;

	gameData_currentRoom = g_rooms[roomNumber];

	if (gameData_currentPlayerData != NULL)
		gameData_currentPlayerData->currentRoom = gameData_currentRoom;

	gameData_currentRoom = g_rooms[TRANSITION_ROOM_INDEX];
	gameData_currentRoom->init(g_rooms[roomNumber]);

	if (Game_ChangedRoomCallback != NULL)
		Game_ChangedRoomCallback(roomNumber, TRANSITION_ROOM_INDEX);
}

void Game_WipeTransitionToRoom(dl_u8 roomNumber)
{
	gameData_transitionRoomNumber = roomNumber;

	gameData_currentRoom = g_rooms[roomNumber];

	if (gameData_currentPlayerData != NULL)
		gameData_currentPlayerData->currentRoom = gameData_currentRoom;

	gameData_currentRoom = g_rooms[WIPE_TRANSITION_ROOM_INDEX];
	gameData_currentRoom->init(g_rooms[roomNumber]);

	if (Game_ChangedRoomCallback != NULL)
		Game_ChangedRoomCallback(roomNumber, WIPE_TRANSITION_ROOM_INDEX);
}
