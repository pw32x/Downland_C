#include "player.h"

#include "draw_utils.h"

#define PLAYER_INACTIVE			0
#define PLAYER_RESETTING_MAYBE	1
#define PLAYER_ACTIVE			2
#define PLAYER_DYING_MAYBE		0xff

#define PLAYER_RUN_SPEED_LEFT	0xffca
#define PLAYER_RUN_SPEED_RIGHT	0x36

#define PLAYER_START_X 0x70 // 112
#define PLAYER_START_Y 0xa5 // 165

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

void Player_Init(PlayerData* playerData, const Resources* resources)
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
											        PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);
}

void Player_Update(PlayerData* playerData, const JoystickState* joystickState, u8* framebuffer, u8* cleanBackground)
{
	eraseSprite_24PixelsWide(framebuffer, 
							 cleanBackground,
							 GET_HIGH_BYTE(playerData->x),
							 GET_HIGH_BYTE(playerData->y),
							 playerData->currentSprite,
							 PLAYER_SPRITE_ROWS);

	if (playerData->state == 0xff)
	{
		playerData->state = 0;
		return;
	}

	playerData->speedx = 0;

	if (joystickState->leftDown)
	{
		playerData->speedx = PLAYER_RUN_SPEED_LEFT;
	}
	else if (joystickState->rightDown)
	{
		playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
	}

	playerData->x += playerData->speedx;
	playerData->y += playerData->speedy;

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
												    playerData->currentFrame,
												    GET_HIGH_BYTE(playerData->x) & 3, 
												    PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);

	drawSprite_24PixelsWide(playerData->currentSprite, 
							GET_HIGH_BYTE(playerData->x), 
							GET_HIGH_BYTE(playerData->y), 
							PLAYER_SPRITE_ROWS, 
							framebuffer);
}