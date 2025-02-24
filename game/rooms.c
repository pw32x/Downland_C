#include "rooms.h"

#include <string.h>

#include "game_types.h"
#include "draw_utils.h"
#include "drops_manager.h"
#include "pickup_types.h"
#include "string_utils.h"
#include "game.h"
#include "ball.h"
#include "bird.h"
#include "player.h"

void titleScreen_draw(u8 roomNumber, GameData* gameData, Resources* resources)
{
	u8* framebuffer = gameData->framebuffer;

	// init background and text
	drawBackground(&resources->roomResources[roomNumber].backgroundDrawData, 
				   resources,
				   framebuffer);

	// title screen text
	drawText(resources->text_downland, resources->characterFont, framebuffer, 0x03c9); // 0x07c9 original coco mem location
	drawText(resources->text_writtenBy, resources->characterFont, framebuffer, 0x050a); // 0x090A original coco mem location
	drawText(resources->text_michaelAichlmayer, resources->characterFont, framebuffer, 0x647); // 0x0A47 original coco mem location
	drawText(resources->text_copyright1983, resources->characterFont, framebuffer, 0x789); // 0x0B89 original coco mem location
	drawText(resources->text_spectralAssociates, resources->characterFont, framebuffer, 0x8c6); // 0x0CC6 original coco mem location
	drawText(resources->text_licensedTo, resources->characterFont, framebuffer, 0xa0a); // 0x0E0A original coco mem location
	drawText(resources->text_tandyCorporation, resources->characterFont, framebuffer, 0xb47); // 0x0F47 original coco mem location
	drawText(resources->text_allRightsReserved, resources->characterFont, framebuffer, 0xc86); // 0x1086 original coco mem location
	drawText(resources->text_onePlayer, resources->characterFont, framebuffer, 0xf05); // 0x1305 original coco mem location
	drawText(resources->text_twoPlayer, resources->characterFont, framebuffer, 0xf11); // 0x1311 original coco mem location
	drawText(resources->text_highScore, resources->characterFont, framebuffer, 0x118b); // 0x158B original coco mem location
	drawText(resources->text_playerOne, resources->characterFont, framebuffer, 0x1406); // 0x1806 original coco mem location
	drawText(resources->text_playerTwo, resources->characterFont, framebuffer, 0x1546); // 0x1946 original coco mem location

	convertScoreToString(gameData->playerOneScore, gameData->string_playerOneScore);

	drawText(gameData->string_playerOneScore, 
			 resources->characterFont, 
			 framebuffer, 
			 TITLESCREEN_PLAYERONE_SCORE_LOCATION);

	convertScoreToString(gameData->playerTwoScore, gameData->string_playerTwoScore);

	drawText(gameData->string_playerTwoScore, 
			 resources->characterFont, 
			 framebuffer, 
			 TITLESCREEN_PLAYERTWO_SCORE_LOCATION);

	convertScoreToString(gameData->highScore, gameData->string_highScore);

	drawText(gameData->string_highScore, 
			 resources->characterFont, 
			 framebuffer, 
			 TITLESCREEN_HIGHSCORE_LOCATION);
}

void titleScreen_init(Room* room, GameData* gameData, Resources* resources)
{
	u8 roomNumber = room->roomNumber;
	gameData->gameCompletionCount = 1; // act like the game was going through one for the title screen

	titleScreen_draw(roomNumber, gameData, resources);

	memcpy(gameData->cleanBackground, gameData->framebuffer, FRAMEBUFFER_SIZE_IN_BYTES);

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[roomNumber].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, roomNumber, gameData->gameCompletionCount);
}

void titleScreen_update(Room* room, GameData* gameData, Resources* resources)
{
	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						gameData->gameCompletionCount,
						resources->sprites_drops);

	if (gameData->joystickState.leftPressed)
	{
		gameData->numPlayers = 1;

	}
	else if (gameData->joystickState.rightPressed)
	{
		gameData->numPlayers = 2;
	}

	u16 drawLocation = gameData->numPlayers == 1 ? 0xf64 : 0xf70;  // hardcoded locations in the frambuffer
	u16 eraseLocation = gameData->numPlayers == 1 ? 0xf70 : 0xf64; // based on the original game.

	gameData->framebuffer[drawLocation] = 0xff;
	gameData->framebuffer[eraseLocation] = 0;

	if (gameData->joystickState.jumpPressed)
	{
		memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);
		Game_TransitionToRoom(gameData, 0, resources);
	}
}

Room titleScreenRoom =
{
	TITLESCREEN_ROOM_INDEX,
	(InitRoomFunctionType)titleScreen_init,
	(DrawRoomFunctionType)titleScreen_draw,
	(UpdateRoomFunctionType)titleScreen_update
};

#define PICKUPS_NUM_SPRITE_ROWS 10

void drawPickups(Pickup* pickups, 
				 u8 currentPlayer,
				 Resources* resources, 
				 u8* framebuffer)
{
	u8 count = NUM_PICKUPS_PER_ROOM;

	while (count--)
	{
		if (!(pickups->state & currentPlayer))
			continue;

		drawSprite_16PixelsWide(resources->pickupSprites[pickups->type],
								pickups->x, 
								pickups->y, 
								PICKUPS_NUM_SPRITE_ROWS,
								framebuffer);
		pickups++;
	}
}

#define PLAYERICON_NUM_SPRITE_ROWS 7

// this is all wrong
void drawPlayerLives(u8 playerLives,
					 u8 currentFrame,
					 u8* playerBitShiftedSprites,
					 u8* framebuffer)
{
	u8 x = PLAYERLIVES_ICON_X;
	u8 y = PLAYERLIVES_ICON_Y;

	u8* currentSprite = getBitShiftedSprite(playerBitShiftedSprites, 
											currentFrame, 
											0, 
											PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);

	for (u8 loop = 0; loop < playerLives; loop++)
	{
		drawSprite_24PixelsWide(currentSprite,
								x, 
								y, 
								PLAYERICON_NUM_SPRITE_ROWS,
								framebuffer);

		x += PLAYERLIVES_ICON_SPACING;
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


void room_draw(u8 roomNumber, GameData* gameData, Resources* resources)
{
	drawBackground(&resources->roomResources[roomNumber].backgroundDrawData, 
				   resources,
				   gameData->cleanBackground);
}

void room_init(Room* room, GameData* gameData, Resources* resources)
{
	u8 roomNumber = room->roomNumber;

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[roomNumber].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, roomNumber, gameData->gameCompletionCount);

	Ball_Init(&gameData->ballData, roomNumber, resources);
	Bird_Init(&gameData->birdData, roomNumber, resources);
	Player_Init(&gameData->playerData, resources);

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

	drawPlayerLives(gameData->playerLives,
					gameData->playerData.currentFrameNumber,
					gameData->playerData.bitShiftedSprites,
					gameData->framebuffer);

	convertScoreToString(gameData->playerOneScore, gameData->string_playerOneScore);

	drawText(gameData->string_playerOneScore, 
			 resources->characterFont, 
			 gameData->framebuffer, 
			 SCORE_DRAW_LOCATION);
}

void room_update(Room* room, GameData* gameData, Resources* resources)
{
	// in the original rom, pickups are indeed drawn every frame
	// otherwise, falling drops will erase them
	drawPickups(gameData->gamePickups[room->roomNumber], 
				gameData->currentPlayer,
				resources, gameData->framebuffer);

	updateTimers(gameData->currentRoom->roomNumber, gameData->roomTimers);
	u16 currentTimer = gameData->roomTimers[gameData->currentRoom->roomNumber];

	Ball_Update(&gameData->ballData, gameData->framebuffer, gameData->cleanBackground);
	Bird_Update(&gameData->birdData, currentTimer, gameData->framebuffer, gameData->cleanBackground);
	Player_Update(&gameData->playerData, &gameData->joystickState, gameData->framebuffer, gameData->cleanBackground);

	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						gameData->gameCompletionCount,
						resources->sprites_drops);	

	convertTimerToString(currentTimer,
						 gameData->string_timer);

	drawText(gameData->string_timer, 
			 resources->characterFont, 
			 gameData->framebuffer, 
			 TIMER_DRAW_LOCATION);
}

Room room0 =
{
	0,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room1 =
{
	1,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room2 =
{
	2,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room3 =
{
	3,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room4 =
{
	4,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room5 =
{
	5,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room6 =
{
	6,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room7 =
{
	7,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room8 =
{
	8,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

Room room9 =
{
	9,
	(InitRoomFunctionType)room_init,
	(DrawRoomFunctionType)room_draw,
	(UpdateRoomFunctionType)room_update
};

void transition_init(Room* targetRoom, GameData* gameData, Resources* resources)
{
	// init the clean background with the target room. 
	// we'll be slowly revealing it during the room transition.
	targetRoom->draw(gameData->transitionRoomNumber, (struct GameData*)gameData, resources);

	// setup screen transition
	gameData->transitionInitialDelay = 30;
	gameData->transitionCurrentLine = 0;
	gameData->transitionFrameDelay = 0;
}

void transition_update(Room* room, GameData* gameData, Resources* resources)
{
	if (gameData->transitionInitialDelay)
	{
		gameData->transitionInitialDelay--;
		return;
	}

	gameData->transitionFrameDelay = !gameData->transitionFrameDelay;

	if (gameData->transitionFrameDelay)
		return;

	u16 offset = gameData->transitionCurrentLine * FRAMEBUFFER_PITCH;

	u8* cleanBackgroundRunner = gameData->cleanBackground + offset;
	u8* framebufferRunner = gameData->framebuffer + offset;

	for (int loop = 0; loop < 6; loop++)
	{
		for (int innerLoop = 0; innerLoop < FRAMEBUFFER_PITCH; innerLoop++)
		{
			*framebufferRunner = *cleanBackgroundRunner;

			if (gameData->transitionCurrentLine < 31)
			{
				*(framebufferRunner + FRAMEBUFFER_PITCH) = CRT_EFFECT_MASK;
			}

			framebufferRunner++;
			cleanBackgroundRunner++;
		}

		cleanBackgroundRunner += 0x3e0;
		framebufferRunner += 0x3e0;
	}

	gameData->transitionCurrentLine++;

	if (gameData->transitionCurrentLine == 32)
	{
		Game_EnterRoom(gameData, gameData->transitionRoomNumber, resources);
	}
}

Room transitionRoom =
{
	TRANSITION_ROOM_INDEX,
	(InitRoomFunctionType)transition_init,
	NULL,
	(UpdateRoomFunctionType)transition_update
};

Room* g_rooms[] = 
{
	&room0,
	&room1,
	&room2,
	&room3,
	&room4,
	&room5,
	&room6,
	&room7,
	&room8,
	&room9,
	&titleScreenRoom,
	&transitionRoom
};