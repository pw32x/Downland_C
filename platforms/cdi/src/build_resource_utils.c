#include "build_resource_utils.h"
#include "game_runner.h"
#include "image_utils.h"

extern GameSprite splatSprite;

void buildSpriteResource(GameSprite* gameSprite,
						 const dl_u8* originalSprite,
						 dl_u8 frameWidth,
						 dl_u8 frameHeight,
						 dl_u8 spriteCount)
{
	dl_u16 spriteDataSize = frameWidth * frameHeight * spriteCount;
	dl_u8* spriteData = (dl_u8*)malloc(spriteDataSize);
	dl_u8* spriteDataRunner;
	const dl_u8* originalSpriteRunner;
	dl_u16 sizePerOriginalSpriteFrame;
	dl_u8 loop;

	memset(spriteData, 0, sizeof(spriteDataSize));

	gameSprite->spriteData = spriteData;
	gameSprite->frameWidth = frameWidth;
	gameSprite->frameHeight = frameHeight;
	gameSprite->sizePerFrame = frameWidth * frameHeight;

	spriteDataRunner = spriteData;
	originalSpriteRunner = originalSprite;

	sizePerOriginalSpriteFrame = (frameWidth / 8) * frameHeight;

	for (loop = 0; loop < spriteCount; loop++)
    {
		convert1bppImageTo8bppCrtEffect(originalSpriteRunner,
											  spriteDataRunner,
											  frameWidth,
											  frameHeight,
											  frameWidth,
											  0);

		spriteDataRunner += gameSprite->sizePerFrame;
		originalSpriteRunner += sizePerOriginalSpriteFrame;
	}
}

void buildTextResource(GameSprite* gameSprite,
					   const dl_u8* originalSprite,
					   dl_u8 frameWidth,
					   dl_u8 frameHeight,
					   dl_u8 spriteCount)
{
	dl_u16 spriteDataSize = frameWidth * frameHeight * spriteCount;
	dl_u8* spriteData = (dl_u8*)malloc(spriteDataSize);
	dl_u8* spriteDataRunner;
	const dl_u8* originalSpriteRunner;
	dl_u16 sizePerOriginalSpriteFrame;
	int loop;
	int loop2;
	dl_u16 frameSize;

	memset(spriteData, 0, sizeof(spriteDataSize));

	gameSprite->spriteData = spriteData;
	gameSprite->frameWidth = frameWidth;
	gameSprite->frameHeight = frameHeight;
	gameSprite->sizePerFrame = frameWidth * frameHeight;

	spriteDataRunner = spriteData;
	originalSpriteRunner = originalSprite;

	sizePerOriginalSpriteFrame = (frameWidth / 8) * frameHeight;

	for (loop = 0; loop < spriteCount; loop++)
    {
		convert1bppImageTo8bppCrtEffect(originalSpriteRunner,
											  spriteDataRunner,
											  frameWidth,
											  frameHeight,
											  frameWidth,
											  4);

		frameSize = frameWidth * frameHeight;
		for (loop2 = 0; loop2 < frameSize; loop2++)
		{
			if (spriteDataRunner[loop2] == 0)
				spriteDataRunner[loop2] = 4;
			if (spriteDataRunner[loop2] == 3)
				spriteDataRunner[loop2] = 1;
		}

		spriteDataRunner += gameSprite->sizePerFrame;
		originalSpriteRunner += sizePerOriginalSpriteFrame;
	}
}

void buildRegenFrame(const dl_u8* originalSprite, 
					  dl_u8* destinationSprite, 
					  dl_u8 currentPlayerSpriteNumber)
{
	const dl_u16 bufferSize = (PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS;
	dl_u8 staticSprite[(PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS];

    memset(staticSprite, 0, bufferSize);

    drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													PLAYER_SPRITE_ROWS,
													staticSprite);

	convert1bppImageTo8bppCrtEffect(staticSprite,
									destinationSprite,
									PLAYER_SPRITE_WIDTH,
									PLAYER_SPRITE_ROWS,
									PLAYER_SPRITE_WIDTH,
									0);

}

void buildRegenSprites(GameSprite* gameSprite,
					   const dl_u8* originalSprite,
					   dl_u8 frameWidth,
					   dl_u8 frameHeight,
					   dl_u8 spriteCount)
{
	dl_u16 sizePerFrame = frameWidth * frameHeight;
	dl_u16 spriteDataSize;
	dl_u8 loop;
	dl_u8* spriteDataRunner;

	gameSprite->frameWidth = frameWidth;
	gameSprite->frameHeight = frameHeight;
	gameSprite->sizePerFrame = sizePerFrame;

	spriteDataSize = frameWidth * frameHeight * (spriteCount * 2); // *2 for left and right
	gameSprite->spriteData = (dl_u8*)malloc(spriteDataSize);

	spriteDataRunner = gameSprite->spriteData;

	// right side
	for (loop = 0; loop < spriteCount; loop++)
	{
		buildRegenFrame(originalSprite, spriteDataRunner, loop);
		spriteDataRunner += sizePerFrame;
	}

	// left side
	originalSprite += ((PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS) * 6; // first frame facing left
	for (loop = 0; loop < spriteCount; loop++)
	{
		buildRegenFrame(originalSprite, spriteDataRunner, loop);
		spriteDataRunner += sizePerFrame;
	}
}



#define SPLAT_FRAME_COUNT 2

void buildSplatSpriteResource(const Resources* resources)
{
	dl_u16 spriteDataSize;
	dl_u8* spriteDataRunner;
	dl_u8 loop;

	splatSprite.frameWidth = PLAYER_SPLAT_SPRITE_WIDTH;
	splatSprite.frameHeight = PLAYER_SPLAT_SPRITE_ROWS;
	splatSprite.sizePerFrame = PLAYER_SPLAT_SPRITE_WIDTH * PLAYER_SPLAT_SPRITE_ROWS;

	spriteDataSize = splatSprite.sizePerFrame * SPLAT_FRAME_COUNT;
	splatSprite.spriteData = (dl_u8*)malloc(spriteDataSize);

	spriteDataRunner = splatSprite.spriteData;

	for (loop = 0; loop < SPLAT_FRAME_COUNT; loop++)
	{
		convert1bppImageTo8bppCrtEffect(resources->sprite_playerSplat,
											  spriteDataRunner,
											  splatSprite.frameWidth,
											  splatSprite.frameHeight,
											  splatSprite.frameWidth,
											  0);

		spriteDataRunner += splatSprite.sizePerFrame;
	}

	spriteDataRunner = splatSprite.spriteData + splatSprite.sizePerFrame;

	for (loop = 0; loop < 5 * splatSprite.frameWidth; loop++)
	{
		spriteDataRunner[loop] = 0;
	}
}
