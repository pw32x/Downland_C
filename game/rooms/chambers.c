#include "room_types.h"

#include "..\game_types.h"
#include "..\draw_utils.h"
#include "..\drops_manager.h"
#include "..\ball.h"
#include "..\bird.h"
#include "..\player.h"
#include "..\door_utils.h"


//#define DISABLE_ENEMIES


void drawPickups(Pickup* pickups, 
				 u8 playerMask,
				 Resources* resources, 
				 u8* framebuffer)
{
	u8 count = NUM_PICKUPS_PER_ROOM;

	while (count--)
	{
		if ((pickups->state & playerMask))
		{
			drawSprite_16PixelsWide(resources->pickupSprites[pickups->type],
									pickups->x, 
									pickups->y, 
									PICKUPS_NUM_SPRITE_ROWS,
									framebuffer);
		}
		pickups++;
	}
}

#define PLAYERICON_NUM_SPRITE_ROWS 7

void drawPlayerLives(u8 playerLives,
					 u8 currentSpriteNumber,
					 u8* playerBitShiftedSprites,
					 u8* framebuffer,
					 u8* cleanBackground,
					 u8 isRegenerating)
{
	u8 x = PLAYERLIVES_ICON_X;
	u8 y = PLAYERLIVES_ICON_Y;

	u8* currentSprite = getBitShiftedSprite(playerBitShiftedSprites, 
											currentSpriteNumber, 
											0, 
											PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);

	for (u8 loop = 0; loop < playerLives; loop++)
	{
		drawSprite_24PixelsWide_noblend(currentSprite,
										x, 
										y, 
										PLAYERICON_NUM_SPRITE_ROWS,
										framebuffer);

		x += PLAYERLIVES_ICON_SPACING;
	}

	eraseSprite_24PixelsWide_simple(x, 
									y, 
									PLAYERICON_NUM_SPRITE_ROWS,
									framebuffer,
									cleanBackground);

	if (isRegenerating)
	{
		drawSprite_24PixelsWide_static(currentSprite,
									   x, 
									   y, 
									   PLAYERICON_NUM_SPRITE_ROWS,
									   framebuffer);
	}
}

void updateTimers(u8 roomNumber, u16* roomTimers)
{
	for (int loop = 0; loop < NUM_ROOMS; loop++)
	{
		if (loop == roomNumber)
		{
			if (*roomTimers)
				(*roomTimers)--;
		}
		else 
		{
			if (*roomTimers < ROOM_TIMER_DEFAULT)
				(*roomTimers)++;
		}

		roomTimers++;
	}
}


void chamber_draw(u8 roomNumber, GameData* gameData, Resources* resources)
{
	drawBackground(&resources->roomResources[roomNumber].backgroundDrawData, 
				   resources,
				   gameData->cleanBackground);

	// draw active doors in the room
	DoorInfoData* doorInfoData = &resources->roomResources[roomNumber].doorInfoData;
	DoorInfo* doorInfoRunner = doorInfoData->doorInfos;
	for (u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
		if ((doorInfoRunner->y != 0xff) &&
			(gameData->currentPlayerData->doorStateData[doorInfoRunner->globalDoorIndex] & 
			 gameData->currentPlayerData->playerMask))
		{
			drawDoor(doorInfoRunner, 
					resources->bitShiftedSprites_door, 
					gameData->framebuffer, 
					gameData->cleanBackground,
					FALSE);
		}

		doorInfoRunner++;
	}
}

void chamber_init(Room* room, GameData* gameData, Resources* resources)
{
	u8 roomNumber = room->roomNumber;
	gameData->targetFps = NORMAL_FPS;
	PlayerData* playerData = gameData->currentPlayerData;

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[roomNumber].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, roomNumber, playerData->gameCompletionCount);

	Ball_Init(&gameData->ballData, roomNumber, resources);
	Bird_Init(&gameData->birdData, roomNumber, resources);
	Player_RoomInit(playerData, resources);

	drawText(resources->text_pl1, 
			 resources->characterFont, 
			 gameData->framebuffer, 
			 PLAYERLIVES_TEXT_DRAW_LOCATION);

	drawText(resources->text_chamber, 
			 resources->characterFont, 
			 gameData->framebuffer, 
			 CHAMBER_TEXT_DRAW_LOCATION);

	gameData->string_roomNumber[0] = roomNumber;

	drawText(gameData->string_roomNumber, 
			 resources->characterFont, 
			 gameData->framebuffer, 
			 CHAMBER_NUMBER_TEXT_DRAW_LOCATION);

	convertScoreToString(playerData->score, playerData->scoreString);

	drawText(playerData->scoreString, 
			 resources->characterFont, 
			 gameData->framebuffer, 
			 SCORE_DRAW_LOCATION);
}

void chamber_update(Room* room, GameData* gameData, Resources* resources)
{
	PlayerData* playerData = gameData->currentPlayerData;

	// in the original rom, pickups are indeed drawn every frame
	// otherwise, falling drops will erase them
	drawPickups(playerData->gamePickups[room->roomNumber], 
				gameData->currentPlayerData->playerMask,
				resources, gameData->framebuffer);

	updateTimers(playerData->currentRoom->roomNumber, playerData->roomTimers);
	u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

#ifndef DISABLE_ENEMIES
	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						playerData->gameCompletionCount,
						resources->sprites_drops);	
#endif

	DoorInfo* lastDoor = playerData->lastDoor;

	u8 playerLives = playerData->lives;

	Player_Update(playerData, 
				  &gameData->joystickState, 
				  gameData->framebuffer, 
				  gameData->cleanBackground,
				  &resources->roomResources[room->roomNumber].doorInfoData,
				  playerData->doorStateData);

	// player lost a life check
	if (playerData->lives < playerLives ||
		playerData->gameOver)
	{
		// if there's another player and they
		// have lives left, then switch.
		if (gameData->otherPlayerData != NULL &&
			!gameData->otherPlayerData->gameOver)
		{
			// switch players
			PlayerData* temp = gameData->otherPlayerData;
			gameData->otherPlayerData = gameData->currentPlayerData;
			gameData->currentPlayerData = temp;

			Game_TransitionToRoom(gameData, 
								  GET_READY_ROOM_INDEX, 
								  resources);
			return;
		}
		else
		{
			// if there's only one player left and
			// they have lives left, then regen.
			// if not, then game over.

			if (playerData->gameOver)
			{
				Game_TransitionToRoom(gameData, 
									  TITLESCREEN_ROOM_INDEX, 
									  resources);
				return;
			}
			else
			{
				Player_StartRegen(playerData);
			}
		}
	}

	if (lastDoor != playerData->lastDoor)
	{
		if (playerData->lastDoor->globalDoorIndex == LAST_DOOR_INDEX)
		{
			Player_CompleteGameLoop(playerData, resources);
		}

		Game_WipeTransitionToRoom(gameData, 
								  playerData->lastDoor->nextRoomNumber, 
								  resources);
		return;
	}

#ifndef DISABLE_ENEMIES
	Ball_Update(&gameData->ballData, gameData->framebuffer, gameData->cleanBackground);
	Bird_Update(&gameData->birdData, currentTimer, gameData->framebuffer, gameData->cleanBackground);
#endif

	if (Player_HasCollision(playerData, gameData->framebuffer, gameData->cleanBackground))
	{
		// compute collisions
		// pick up item or die
		int a = 3;
		Player_PerformCollisions((struct GameData*)gameData, resources);
	}

	convertTimerToString(currentTimer,
						 gameData->string_timer);

	drawText(gameData->string_timer, 
			 resources->characterFont, 
			 gameData->framebuffer, 
			 TIMER_DRAW_LOCATION);

	drawPlayerLives(playerData->lives,
					playerData->currentSpriteNumber,
					playerData->bitShiftedSprites,
					gameData->framebuffer,
					gameData->cleanBackground,
					playerData->regenerationCounter > 0);
}

// All the chambers are the same, but this makes it easier
// to index them in g_rooms like any other screen.

Room chamber0 =
{
	0,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber1 =
{
	1,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber2 =
{
	2,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber3 =
{
	3,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber4 =
{
	4,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber5 =
{
	5,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber6 =
{
	6,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber7 =
{
	7,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber8 =
{
	8,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber9 =
{
	9,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};
