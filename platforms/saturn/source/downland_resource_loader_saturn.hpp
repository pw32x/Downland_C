#ifndef RESOURCE_LOADER_SATURN_INCLUDE_H
#define RESOURCE_LOADER_SATURN_INCLUDE_H

#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "resource_types.h"
#include "base_defines.h"
#include "alloc.h"
}

#define RESULT_OK				0
#define RESULT_FILENOTFOUND		1
#define RESULT_CHECKSUMFAILED	2
#define RESULT_UNKNOWNFAILURE	0xff

class DownlandResourceLoader
{
public:
	static int LoadResources(const char* romPath, Resources* resources)
	{
		u8* fileBuffer = NULL;
		u32 fileSize = 0;
		
		if (!loadFile(romPath, &fileBuffer, &fileSize))
			return RESULT_FILENOTFOUND;

		if (fileBuffer == NULL)
			return RESULT_FILENOTFOUND;

		if (!checksumCheck(fileBuffer, fileSize))
			return RESULT_CHECKSUMFAILED;

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
		// pre-build them because gigs of ram. 
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

		resources->roomPickupPositions = (PickupPosition*)getBytes(fileBuffer, 0xd1ea);

		resources->keyPickUpDoorIndexes = getBytes(fileBuffer, 0xd1c2);  // 20 items
		resources->keyPickUpDoorIndexesHardMode = getBytes(fileBuffer, 0xd1d6);  // 20 items
		resources->offsetsToDoorsAlreadyActivated = getBytes(fileBuffer, 0xceea);  // 16 items

		resources->roomsWithBouncingBall = getBytes(fileBuffer, 0xceac);

		return RESULT_OK;
	}

private:
	static bool loadFile(const char* romPath, u8** fileBuffer, u32* fileSize)
	{
		SRL::Cd::File file = SRL::Cd::File(romPath);

		if (!file.Exists())
		{
			return false;
		}

		*fileSize = file.Size.Bytes;
		file.Open();

		*fileBuffer = (u8*)dl_alloc(*fileSize);
		file.Read(*fileSize, *fileBuffer);

		file.Close();

		return true;
	}

	static u32 swap_endian_32(u32 val) 
	{
		return ((val >> 24) & 0xFF) | 
			   ((val >> 8) & 0xFF00) | 
			   ((val << 8) & 0xFF0000) | 
			   ((val << 24) & 0xFF000000);
	}

	// Verifies the checksum for Downland V1.1
	static bool checksumCheck(const u8* fileBuffer, u32 fileSize) 
	{
		u32 accumulator = 0;
		u32 value = 0;
		const u32* fileBufferRunner = (const u32*)fileBuffer;

		int loopCount = fileSize / sizeof(value);
		for (int loop = 0; loop < loopCount; loop++)
		{
			value = swap_endian_32(*fileBufferRunner);
			accumulator += value;
			fileBufferRunner++;
		}
		
		return (accumulator == 0x84883253);
	}

	static const u8* getBytes(const u8* fileBuffer, u16 start)
	{
		// take into account that the rom starts at c000
		start -= 0xc000; 

		return fileBuffer + start;
	}

	static u8* getBytesSwapped(const u8* fileBuffer, u16 start, u16 end)
	{
		// take into account that the rom starts at c000
		start -= 0xc000; 
		end -= 0xc000;

		u16 size = end - start;

		u8* buffer = (u8*)dl_alloc(size);
		fileBuffer += start;

		// swap bytes because endianness difference
		u8* bufferRunner = buffer;
		for (int loop = 0; loop < size / 2; loop++)
		{
			bufferRunner[0] = fileBuffer[1];
			bufferRunner[1] = fileBuffer[0];
			bufferRunner += 2;
			fileBuffer += 2;
		}

		return buffer;
	}



	static void loadBackgroundDrawData(const u8* fileBuffer, 
									   u16 start, 
									   BackgroundDrawData* backgroundDrawData)
	{
		// take into account that the rom starts at c000
		start -= 0xc000; 

		fileBuffer += start;

		u8 sentinelValue = 0xff;

		// go through the fileBuffer, counting the number of elements
		// we need to create.
		u8 drawCommandCount = 0;
		const u8* fileBufferRunner = fileBuffer;
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
		BackgroundDrawCommand* backgroundDrawCommands = (BackgroundDrawCommand*)dl_alloc(drawCommandCount * sizeof(BackgroundDrawCommand));

		// fill the background draw commands array
		fileBufferRunner = fileBuffer;
		BackgroundDrawCommand* backgroundDrawCommandsRunner = backgroundDrawCommands;

		u8 repeatCodeIndicator = 0x80; // a shape code with this flag means that it repeats

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
	
	static void loadShapeDrawData(const u8* fileBuffer, 
								  u16 start, 
								  ShapeDrawData* shapeDrawData)
	{
		// take into account that the rom starts at c000
		start -= 0xc000; 

		fileBuffer += start;

		shapeDrawData->segmentCount = *fileBuffer;

		fileBuffer++;

		shapeDrawData->segments = (const ShapeSegment*)fileBuffer;
	}
	
	static void loadDropSpawnPositions(const u8* fileBuffer, 
									   u16 start, 
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
	
	static void loadDoorInfoDataPositions(const u8* fileBuffer, u16 start, DoorInfoData* doorInfoData)
	{
		// take into account that the rom starts at c000
		start -= 0xc000; 
		fileBuffer += start;

		u8 doorInfosCount = 0;
		u8 sentinelValue = 1; // some initial non-zero value

		const u8* fileBufferRunner = fileBuffer;

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
	static u8* buildBitShiftedSprites(const u8* spriteData, u8 spriteCount, u8 rowCount, u8 bytesPerRow)
	{
	#define DESTINATION_BYTES_PER_ROW	3
	#define NUM_BIT_SHIFTS 4

		u16 bitShiftedSpriteBufferSize = spriteCount * rowCount * DESTINATION_BYTES_PER_ROW * NUM_BIT_SHIFTS;
		u8* bitShiftedSprites = (u8*)dl_alloc(bitShiftedSpriteBufferSize);
		u8* bitShiftedSpritesRunner = bitShiftedSprites;

		u32 workBuffer;

		for (int loop = 0; loop < spriteCount; loop++)
		{
			for (int shiftAmount = 0; shiftAmount < NUM_BIT_SHIFTS; shiftAmount++)
			{
				const u8* spriteDataRunner = spriteData + (loop * rowCount * bytesPerRow);

				for (int rowLoop = 0; rowLoop < rowCount; rowLoop++)
				{
					workBuffer = spriteDataRunner[0] << 16 | spriteDataRunner[1] << 8;
					workBuffer >>= (shiftAmount * 2);
					bitShiftedSpritesRunner[0] = (u8)(workBuffer >> 16);
					bitShiftedSpritesRunner[1] = (u8)(workBuffer >> 8);
					bitShiftedSpritesRunner[2] = (u8)workBuffer;

					spriteDataRunner += bytesPerRow;
					bitShiftedSpritesRunner += DESTINATION_BYTES_PER_ROW;
				}
			}
		}

		return bitShiftedSprites;
	}

};

#endif 