#include "chambers.h"
#include "room_types.h"

#include "..\game_types.h"
#include "..\draw_utils.h"
#include "..\drops_manager.h"
#include "..\ball.h"
#include "..\bird.h"
#include "..\player.h"
#include "..\door_utils.h"

void drawPickups(Pickup* pickups, 
				 dl_u8 playerMask,
				 const Resources* resources, 
				 dl_u8* framebuffer)
{
	dl_u8 count = NUM_PICKUPS_PER_ROOM;

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

void drawPlayerLives(dl_u8 playerLives,
					 dl_u8 currentSpriteNumber,
					 const dl_u8* playerBitShiftedSprites,
					 dl_u8* framebuffer,
					 dl_u8* cleanBackground,
					 dl_u8 isRegenerating)
{
	dl_u8 x = PLAYERLIVES_ICON_X;
	dl_u8 y = PLAYERLIVES_ICON_Y;
	dl_u8 loop;

	const dl_u8* currentSprite = getBitShiftedSprite(playerBitShiftedSprites, 
											currentSpriteNumber, 
											0, 
											PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);

	for (loop = 0; loop < playerLives; loop++)
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

void updateTimers(dl_u8 roomNumber, dl_u16* roomTimers)
{
	int loop;

	for (loop = 0; loop < NUM_ROOMS; loop++)
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


void chamber_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
#ifndef DISABLE_DOOR_DRAWING
	const DoorInfoData* doorInfoData;
	const DoorInfo* doorInfoRunner;
	dl_u8 loop;
#endif

	drawBackground(&resources->roomResources[roomNumber].backgroundDrawData, 
				   resources,
				   gameData->cleanBackground);

#ifndef DISABLE_DOOR_DRAWING
	// draw active doors in the room
	doorInfoData = &resources->roomResources[roomNumber].doorInfoData;
	doorInfoRunner = doorInfoData->doorInfos;
	for (loop = 0; loop < doorInfoData->drawInfosCount; loop++)
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
#endif
}

void chamber_init(Room* room, GameData* gameData, const Resources* resources)
{
	dl_u8 roomNumber = room->roomNumber;
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

void chamber_update(Room* room, GameData* gameData, const Resources* resources)
{
	PlayerData* playerData = gameData->currentPlayerData;
	dl_u16 currentTimer;
	const DoorInfo* lastDoor;
	dl_u8 playerLives;
	PlayerData* temp;

	// in the original rom, pickups are indeed drawn every frame
	// otherwise, falling drops will erase them
	drawPickups(playerData->gamePickups[room->roomNumber], 
				gameData->currentPlayerData->playerMask,
				resources, 
				gameData->framebuffer);

	if (playerData->state != PLAYER_STATE_REGENERATION)
	{
		updateTimers(playerData->currentRoom->roomNumber, playerData->roomTimers);
	}

	currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

#ifndef DISABLE_ENEMIES
	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						playerData->gameCompletionCount,
						resources->sprites_drops);	
#endif

	lastDoor = playerData->lastDoor;

	playerLives = playerData->lives;

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
			temp = gameData->otherPlayerData;
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
