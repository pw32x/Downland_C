#include "resource_loader_buffer.h"

#include "../game/base_defines.h"
#include "../game/alloc.h"



static const dl_u8* getBytes(const dl_u8* fileBuffer, dl_u16 start)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 

	return fileBuffer + start;
}

static void loadBackgroundDrawData(const dl_u8* fileBuffer, 
								   dl_u16 start, 
								   BackgroundDrawData* backgroundDrawData)
{
	dl_u8 sentinelValue = 0xff;
	dl_u8 drawCommandCount = 0;
	const dl_u8* fileBufferRunner;
	BackgroundDrawCommand* backgroundDrawCommands;
	BackgroundDrawCommand* backgroundDrawCommandsRunner;
	dl_u8 repeatCodeIndicator;

	// take into account that the rom starts at c000
	start -= 0xc000; 

	fileBuffer += start;
	fileBufferRunner = fileBuffer;

	// go through the fileBuffer, counting the number of elements
	// we need to create.

	while (*fileBufferRunner != sentinelValue)
	{
		drawCommandCount++;

		// if the draw shape code is prefixed with 0x80, then 
		// it's meant to be repeated. Skip the next byte because
		// it's the count.
		if ((*fileBufferRunner & 0x80) != 0)
			fileBufferRunner++;

		fileBufferRunner++;
	}

	// create the list of background draw commands
	backgroundDrawCommands = (BackgroundDrawCommand*)dl_alloc(drawCommandCount * sizeof(BackgroundDrawCommand));

	// fill the background draw commands array
	fileBufferRunner = fileBuffer;
	backgroundDrawCommandsRunner = backgroundDrawCommands;

	repeatCodeIndicator = 0x80; // a shape code with this flag means that it repeats

	while (*fileBufferRunner != sentinelValue)
	{
		if ((*fileBufferRunner & repeatCodeIndicator) != 0)
		{
			backgroundDrawCommandsRunner->shapeCode = (*fileBufferRunner) & ~repeatCodeIndicator;
			fileBufferRunner++;		
			backgroundDrawCommandsRunner->drawCount = *fileBufferRunner;
		}
		else
		{
			backgroundDrawCommandsRunner->shapeCode = *fileBufferRunner;
			backgroundDrawCommandsRunner->drawCount = 1;
		}

		fileBufferRunner++;
		backgroundDrawCommandsRunner++;
	}

	backgroundDrawData->drawCommandCount = drawCommandCount;
	backgroundDrawData->backgroundDrawCommands = backgroundDrawCommands;
}
	
static void loadShapeDrawData(const dl_u8* fileBuffer, 
							  dl_u16 start, 
							  ShapeDrawData* shapeDrawData)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 

	fileBuffer += start;

	shapeDrawData->segmentCount = *fileBuffer;

	fileBuffer++;

	shapeDrawData->segments = (const ShapeSegment*)fileBuffer;
}
	
static void loadDropSpawnPositions(const dl_u8* fileBuffer, 
								   dl_u16 start, 
								   DropSpawnPositions* dropSpawnPositions)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 

	fileBuffer += start;
	dropSpawnPositions->spawnAreasCount = *fileBuffer;

	// in the data, the count is off by one.
	dropSpawnPositions->spawnAreasCount++;

	fileBuffer++;

	dropSpawnPositions->dropSpawnAreas = (const DropSpawnArea*)fileBuffer;
}
	
static void loadDoorInfoDataPositions(const dl_u8* fileBuffer, 
									  dl_u16 start, 
									  DoorInfoData* doorInfoData)
{
	dl_u8 doorInfosCount = 0;
	dl_u8 sentinelValue = 1; // some initial non-zero value

	const dl_u8* fileBufferRunner;

	// take into account that the rom starts at c000
	start -= 0xc000; 
	fileBuffer += start;
	fileBufferRunner = fileBuffer;

	// get the number of doors by going through
	// the file until we hit 0
	while (sentinelValue)
	{
		sentinelValue = *fileBufferRunner;

		if (!sentinelValue)
		{
			break;
		}

		// move ahead by one DoorInfo
		fileBufferRunner += sizeof(DoorInfo); 
		doorInfosCount++;
	}

	if (!doorInfosCount) // this would be weird
		return;

	doorInfoData->drawInfosCount = doorInfosCount;
	doorInfoData->doorInfos = (const DoorInfo*)fileBuffer;
}
	
#define WIDTH_BYTES 2  // Original sprite width in bytes
#define HEIGHT 10      // Number of rows
#define SHIFT_COUNT 3  // Number of shifted versions

// assume two bytes per row
static dl_u8* buildBitShiftedSprites(const dl_u8* spriteData, 
									 dl_u8 spriteCount, 
									 dl_u8 rowCount, 
									 dl_u8 bytesPerRow)
{
#define DESTINATION_BYTES_PER_ROW	3
#define NUM_BIT_SHIFTS 4

	dl_u16 bitShiftedSpriteBufferSize = spriteCount * rowCount * DESTINATION_BYTES_PER_ROW * NUM_BIT_SHIFTS;
	dl_u8* bitShiftedSprites = (dl_u8*)dl_alloc(bitShiftedSpriteBufferSize);
	dl_u8* bitShiftedSpritesRunner = bitShiftedSprites;

	dl_u32 workBuffer;
	int loop;
	int shiftAmount;
	const dl_u8* spriteDataRunner;
	int rowLoop;

	for (loop = 0; loop < spriteCount; loop++)
	{
		for (shiftAmount = 0; shiftAmount < NUM_BIT_SHIFTS; shiftAmount++)
		{
			spriteDataRunner = spriteData + (loop * rowCount * bytesPerRow);

			for (rowLoop = 0; rowLoop < rowCount; rowLoop++)
			{
				workBuffer = spriteDataRunner[0] << 16 | spriteDataRunner[1] << 8;
				workBuffer >>= (shiftAmount * 2);
				bitShiftedSpritesRunner[0] = (dl_u8)(workBuffer >> 16);
				bitShiftedSpritesRunner[1] = (dl_u8)(workBuffer >> 8);
				bitShiftedSpritesRunner[2] = (dl_u8)workBuffer;

				spriteDataRunner += bytesPerRow;
				bitShiftedSpritesRunner += DESTINATION_BYTES_PER_ROW;
			}
		}
	}

	return bitShiftedSprites;
}


dl_u8 ResourceLoaderBuffer_Init(const dl_u8* fileBuffer, 
							    dl_u32 fileBufferSize, 
							    Resources* resources)
{
	if (fileBufferSize != DOWNLAND_ROM_SIZE)
		return FALSE;

	// get character font
	resources->characterFont = getBytes(fileBuffer, 0xd908);

	// get strings
	resources->text_downland			= getBytes(fileBuffer, 0xda19);
	resources->text_writtenBy			= getBytes(fileBuffer, 0xda27);
	resources->text_michaelAichlmayer	= getBytes(fileBuffer, 0xda33);
	resources->text_copyright1983		= getBytes(fileBuffer, 0xda45);
	resources->text_spectralAssociates	= getBytes(fileBuffer, 0xda54);
	resources->text_licensedTo			= getBytes(fileBuffer, 0xda68);
	resources->text_tandyCorporation	= getBytes(fileBuffer, 0xda75);
	resources->text_allRightsReserved	= getBytes(fileBuffer, 0xda87);
	resources->text_onePlayer			= getBytes(fileBuffer, 0xda9b);
	resources->text_twoPlayer			= getBytes(fileBuffer, 0xdaa6);
	resources->text_highScore			= getBytes(fileBuffer, 0xdab1);
	resources->text_playerOne			= getBytes(fileBuffer, 0xdabc);
	resources->text_playerTwo			= getBytes(fileBuffer, 0xdac7);
	resources->text_pl1					= getBytes(fileBuffer, 0xdad2);
	resources->text_pl2					= getBytes(fileBuffer, 0xdad6);
	resources->text_getReadyPlayerOne	= getBytes(fileBuffer, 0xdada);
	resources->text_getReadyPlayerTwo	= getBytes(fileBuffer, 0xdaef);
	resources->text_chamber				= getBytes(fileBuffer, 0xdb04);

		
	// get sprites
	resources->sprites_player			= getBytes(fileBuffer, 0xdcd7);
	resources->collisionmasks_player	= getBytes(fileBuffer, 0xde17);
	resources->sprites_bouncyBall		= getBytes(fileBuffer, 0xde7b);
	resources->sprites_bird				= getBytes(fileBuffer, 0xde9b);
	resources->sprite_moneyBag			= getBytes(fileBuffer, 0xdeb3);
	resources->sprite_diamond			= getBytes(fileBuffer, 0xdec7);
	resources->sprite_key				= getBytes(fileBuffer, 0xdedb);
	resources->sprite_playerSplat		= getBytes(fileBuffer, 0xdeef);
	resources->sprite_door				= getBytes(fileBuffer, 0xdf0a);
	resources->sprites_drops			= getBytes(fileBuffer, 0xdf2a);
		
	// generate bit shifted sprites
	resources->bitShiftedSprites_player = buildBitShiftedSprites(resources->sprites_player, PLAYER_SPRITE_COUNT, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_BYTES_PER_ROW);
	resources->bitShiftedCollisionmasks_player = buildBitShiftedSprites(resources->collisionmasks_player, PLAYER_SPRITE_COUNT, PLAYER_COLLISION_MASK_ROWS, PLAYER_COLLISION_MASK_BYTES_PER_ROW);
	resources->bitShiftedSprites_bouncyBall = buildBitShiftedSprites(resources->sprites_bouncyBall, BALL_SPRITE_COUNT, BALL_SPRITE_ROWS, BALL_SPRITE_BYTES_PER_ROW);
	resources->bitShiftedSprites_bird = buildBitShiftedSprites(resources->sprites_bird, BIRD_SPRITE_COUNT, BIRD_SPRITE_ROWS, BIRD_SPRITE_BYTES_PER_ROW);

	// in the original game, the player splat sprite and the door
	// are loaded, bitshifted, and drawn on demand. Here we just 
	// pre-build them because megs of ram. 
	resources->bitShiftedSprites_playerSplat = buildBitShiftedSprites(resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_COUNT, PLAYER_SPLAT_SPRITE_ROWS, PLAYER_SPLAT_SPRITE_BYTES_PER_ROW);
	resources->bitShiftedSprites_door = buildBitShiftedSprites(resources->sprite_door, DOOR_SPRITE_COUNT, DOOR_SPRITE_ROWS, DOOR_SPRITE_BYTES_PER_ROW);

	resources->pickupSprites[0] = resources->sprite_diamond;
	resources->pickupSprites[1] = resources->sprite_moneyBag;
	resources->pickupSprites[2] = resources->sprite_key;

	// get shapes data
	loadShapeDrawData(fileBuffer, 0xd5f7, &resources->shapeDrawData_00_Stalactite);
	loadShapeDrawData(fileBuffer, 0xd60c, &resources->shapeDrawData_01_WallGoingDown);
	loadShapeDrawData(fileBuffer, 0xd616, &resources->shapeDrawData_07_WallPieceGoingUp);
	loadShapeDrawData(fileBuffer, 0xd625, &resources->shapeDrawData_02_LeftHandCornerPiece);
	loadShapeDrawData(fileBuffer, 0xd635, &resources->shapeDrawData_08_CornerPieceGoingDownLeft);
	loadShapeDrawData(fileBuffer, 0xd644, &resources->shapeDrawData_03_TopRightHandCornerPiece);
	loadShapeDrawData(fileBuffer, 0xd654, &resources->shapeDrawData_04_TopRightHandCornerPiece2);
	loadShapeDrawData(fileBuffer, 0xd663, &resources->shapeDrawData_05_BottomRightSideOfFloatingPlatforms);
	loadShapeDrawData(fileBuffer, 0xd67b, &resources->shapeDrawData_14_HorizontalRopeStartGoingRight);
	loadShapeDrawData(fileBuffer, 0xd68d, &resources->shapeDrawData_15_HorizontalRopeEndGoingRight);
	loadShapeDrawData(fileBuffer, 0xd697, &resources->shapeDrawData_17_BlankAreaGoingRight);
	loadShapeDrawData(fileBuffer, 0xd6a0, &resources->shapeDrawData_18_BlankAreaGoingLeft);
	loadShapeDrawData(fileBuffer, 0xd6a9, &resources->shapeDrawData_19_BlankAreaGoingDownRight);
	loadShapeDrawData(fileBuffer, 0xd6b2, &resources->shapeDrawData_0b_ShortLineGoingUp);
	loadShapeDrawData(fileBuffer, 0xd6d9, &resources->shapeDrawData_0c_VeryShortRope);
	loadShapeDrawData(fileBuffer, 0xd6e5, &resources->shapeDrawData_0d_ShortRope);
	loadShapeDrawData(fileBuffer, 0xd6f1, &resources->shapeDrawData_0e_MidLengthRope);
	loadShapeDrawData(fileBuffer, 0xd6fd, &resources->shapeDrawData_0f_LongRope);
	loadShapeDrawData(fileBuffer, 0xd709, &resources->shapeDrawData_10_VeryLongRope);
	loadShapeDrawData(fileBuffer, 0xd715, &resources->shapeDrawData_11_SuperLongRope);
	loadShapeDrawData(fileBuffer, 0xd721, &resources->shapeDrawData_12_ExcessivelyLongRope);
	loadShapeDrawData(fileBuffer, 0xd72d, &resources->shapeDrawData_13_RediculouslyLongRope);
	loadShapeDrawData(fileBuffer, 0xd74c, &resources->shapeDrawData_PreRope_Maybe);
	loadShapeDrawData(fileBuffer, 0xd750, &resources->shapeDrawData_PostRope_Maybe);

	// get background resources
	loadBackgroundDrawData(fileBuffer, 0xd35e, &resources->roomResources[0].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd3a0, &resources->roomResources[1].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd3e4, &resources->roomResources[2].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd426, &resources->roomResources[3].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd467, &resources->roomResources[4].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd4a2, &resources->roomResources[5].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd4d9, &resources->roomResources[6].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd4f6, &resources->roomResources[7].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd52f, &resources->roomResources[8].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xd561, &resources->roomResources[9].backgroundDrawData);
	loadBackgroundDrawData(fileBuffer, 0xcec4, &resources->roomResources[10].backgroundDrawData);

	loadDropSpawnPositions(fileBuffer, 0xd073, &resources->roomResources[0].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd089, &resources->roomResources[1].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd09c, &resources->roomResources[2].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd0ac, &resources->roomResources[3].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd0bf, &resources->roomResources[4].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd0cf, &resources->roomResources[5].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd0df, &resources->roomResources[6].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd0e6, &resources->roomResources[7].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd0fc, &resources->roomResources[8].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd10c, &resources->roomResources[9].dropSpawnPositions);
	loadDropSpawnPositions(fileBuffer, 0xd116, &resources->roomResources[10].dropSpawnPositions);

	loadDoorInfoDataPositions(fileBuffer, 0xd270, &resources->roomResources[0].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd27f, &resources->roomResources[1].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd29a, &resources->roomResources[2].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd2c1, &resources->roomResources[3].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd2ee, &resources->roomResources[4].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd309, &resources->roomResources[5].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd31e, &resources->roomResources[6].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd32d, &resources->roomResources[7].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd33c, &resources->roomResources[8].doorInfoData);
	loadDoorInfoDataPositions(fileBuffer, 0xd34b, &resources->roomResources[9].doorInfoData);

	resources->roomPickupPositions = (PickupPosition*)getBytes(fileBuffer, 0xd1ea); // 50 items

	resources->keyPickUpDoorIndexes = getBytes(fileBuffer, 0xd1c2);  // 20 items
	resources->keyPickUpDoorIndexesHardMode = getBytes(fileBuffer, 0xd1d6);  // 20 items
	resources->offsetsToDoorsAlreadyActivated = getBytes(fileBuffer, 0xceea);  // 16 items

	resources->roomsWithBouncingBall = getBytes(fileBuffer, 0xceac); // 9 items

	return TRUE;
}

void ResourceLoaderBuffer_Shutdown(Resources* resources)
{
	// free shapes data
}

