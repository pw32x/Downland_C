#include "player.h"

#include "draw_utils.h"

#define PLAYER_INACTIVE			0
#define PLAYER_RESETTING_MAYBE	1
#define PLAYER_ACTIVE			2
#define PLAYER_DYING_MAYBE		0xff

#define PLAYER_START_X 0x70 // 112
#define PLAYER_START_Y 0xa5 // 165

#define BITSHIFTED_SPRITE_FRAME_SIZE (PLAYER_SPRITE_ROWS * 3) // rows * 3 bytes per row

#define PLAYERSPRITE_RIGHT_STAND		0
#define PLAYERSPRITE_RIGHT_RUN0			1
#define PLAYERSPRITE_RIGHT_RUN1_JUMP	2
#define PLAYERSPRITE_RIGHT_RUN2			3
#define PLAYERSPRITE_RIGHT_CLIMB		4
#define PLAYERSPRITE_LEFT_CLIMB			5
#define PLAYERSPRITE_LEFT_STAND			6
#define PLAYERSPRITE_LEFT_RUN0			7
#define PLAYERSPRITE_LEFT_RUN1_JUMP		8
#define PLAYERSPRITE_LEFT_RUN2			9

void Player_Init(PlayerData* playerData, Resources* resources)
{
	playerData->state = PLAYER_RESETTING_MAYBE;
	playerData->x = SET_HIGH_BYTE(PLAYER_START_X);
	playerData->y = SET_HIGH_BYTE(PLAYER_START_Y);
	playerData->speedx = 0xffa8;
	playerData->speedy = 0;
	playerData->currentFrame = PLAYERSPRITE_LEFT_STAND;

	playerData->bitShiftedSprites = resources->bitShiftedSprites_player;

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
											        playerData->currentFrame,
											        PLAYER_START_X & 3,
											        BITSHIFTED_SPRITE_FRAME_SIZE);
}

void Player_Update(PlayerData* playerData, u8* framebuffer, u8* cleanBackground)
{
	eraseSprite_24PixelsWide(framebuffer, 
							 cleanBackground,
							 GET_HIGH_BYTE(playerData->x),
							 GET_HIGH_BYTE(playerData->y),
							 playerData->currentSprite,
							 PLAYER_SPRITE_ROWS);

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
												    playerData->currentFrame,
												    GET_HIGH_BYTE(playerData->x) & 3, 
												    BITSHIFTED_SPRITE_FRAME_SIZE);

	if (playerData->state == 0xff)
	{
		playerData->state = 0;
		return;
	}

	drawSprite_24PixelsWide(playerData->currentSprite, 
							GET_HIGH_BYTE(playerData->x), 
							GET_HIGH_BYTE(playerData->y), 
							PLAYER_SPRITE_ROWS, 
							framebuffer);
}