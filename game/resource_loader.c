#include "resource_loader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "base_defines.h"

// Checks for Downland V1.1
BOOL checksumCheck(const char* romPath)
{
	u32 accumulator = 0;
	u32 value;

	FILE* file = fopen(romPath, "rb");

	while (fread(&value, sizeof(value), 1, file))
	{
		accumulator += value;
	}

	fclose(file);

	return accumulator == 0x84883253;
}

byte* getBytes(FILE* file, u16 start, u16 end)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 
	end -= 0xc000;

	u16 size = end - start;

	byte* memory = (byte*)malloc(size);

	if (memory == NULL)
		return NULL;

	fseek(file, start, SEEK_SET);
	fread(memory, size, 1, file);

	return memory;
}

byte* getBytesSwapped(FILE* file, u16 start, u16 end)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 
	end -= 0xc000;

	u16 size = end - start;

	byte* memory = (byte*)malloc(size);

	if (memory == NULL)
		return NULL;

	fseek(file, start, SEEK_SET);
	fread(memory, size, 1, file);

	// swap bytes because endianness difference between 6809 and x86
	u8* memoryRunner = memory;
	for (int loop = 0; loop < size / 2; loop++)
	{
		u8 temp = memoryRunner[0];
		memoryRunner[0] = memoryRunner[1];
		memoryRunner[1] = temp;
		memoryRunner += 2;
	}


	return memory;
}


byte* getBytesUntilSentinel(FILE* file, u16 start, u8 sentinelValue, u16* bufferSize)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 

	fseek(file, start, SEEK_SET);

	*bufferSize = 0xffff;
	u8 readCount = 0;
	u8 value;

	while (fread(&value, sizeof(value), 1, file))
	{
		readCount++;
		if (value == sentinelValue)
		{
			byte* memory = (byte*)malloc(readCount);

			if (memory == NULL)
				return NULL;

			fseek(file, start, SEEK_SET);
			fread(memory, readCount, 1, file);

			*bufferSize = readCount;

			return memory;
		}
	}

	return FALSE;
}

void loadBackgroundDrawData(FILE* file, u16 start, BackgroundDrawData* backgroundDrawData)
{
	u8 sentinelValue = 0xff;

	// get the draw buffer
	u16 bufferSize;
	byte* rawBuffer = getBytesUntilSentinel(file, start, sentinelValue, &bufferSize);

	if (rawBuffer == NULL)
		return;

	// go through the buffer, counting the number of elements
	// we need to create.
	u8 drawCommandCount = 0;
	byte* rawBufferRunner = rawBuffer;
	while (*rawBufferRunner != sentinelValue)
	{
		drawCommandCount++;

		// if the draw shape code is prefixed with 0x80, then 
		// it's meant to be repeated. Skip the next byte because
		// it's the count.
		if ((*rawBufferRunner & 0x80) != 0)
			rawBufferRunner++;

		rawBufferRunner++;
	}

	// create the list of background draw commands
	BackgroundDrawCommand* backgroundDrawCommands = (BackgroundDrawCommand*)malloc(drawCommandCount * sizeof(BackgroundDrawCommand));

	// fill the background draw commands array
	rawBufferRunner = rawBuffer;
	BackgroundDrawCommand* backgroundDrawCommandsRunner = backgroundDrawCommands;

	u8 repeatCodeIndicator = 0x80; // a shape code with this flag means that it repeats

	while (*rawBufferRunner != sentinelValue)
	{
		if ((*rawBufferRunner & repeatCodeIndicator) != 0)
		{
			backgroundDrawCommandsRunner->shapeCode = (*rawBufferRunner) & ~repeatCodeIndicator;
			rawBufferRunner++;		
			backgroundDrawCommandsRunner->drawCount = *rawBufferRunner;
		}
		else
		{
			backgroundDrawCommandsRunner->shapeCode = *rawBufferRunner;
			backgroundDrawCommandsRunner->drawCount = 1;
		}

		rawBufferRunner++;
		backgroundDrawCommandsRunner++;
	}

	backgroundDrawData->drawCommandCount = drawCommandCount;
	backgroundDrawData->backgroundDrawCommands = backgroundDrawCommands;

	free(rawBuffer);
}

void loadShapeDrawData(FILE* file, u16 start, ShapeDrawData* shapeDrawData)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 

	fseek(file, start, SEEK_SET);
	fread(&shapeDrawData->segmentCount, sizeof(shapeDrawData->segmentCount), 1, file);

	u16 bufferSize = shapeDrawData->segmentCount * sizeof(ShapeSegment);
	ShapeSegment* segmentsMemory = (ShapeSegment*)malloc(bufferSize);

	if (segmentsMemory == NULL)
		return;

	fread(segmentsMemory, bufferSize, 1, file);

	shapeDrawData->segments = segmentsMemory;
}

void loadDropSpawnPositions(FILE* file, u16 start, DropSpawnPositions* dropSpawnPositions)
{
	// take into account that the rom starts at c000
	start -= 0xc000; 

	fseek(file, start, SEEK_SET);
	fread(&dropSpawnPositions->spawnAreasCount, sizeof(dropSpawnPositions->spawnAreasCount), 1, file);

	// in the data, the count is off by one.
	dropSpawnPositions->spawnAreasCount++;

	u16 bufferSize = dropSpawnPositions->spawnAreasCount * sizeof(ShapeSegment);
	DropSpawnArea* dropSpawnAreasMemory = (DropSpawnArea*)malloc(bufferSize);

	if (dropSpawnAreasMemory == NULL)
		return;

	fread(dropSpawnAreasMemory, bufferSize, 1, file);

	dropSpawnPositions->dropSpawnAreas = dropSpawnAreasMemory;
}

BOOL ResourceLoader_Init(const char* romPath, Resources* resources)
{
	if (!checksumCheck(romPath))
		return FALSE;

	FILE* file = fopen(romPath, "rb");

	if (file == NULL)
		return FALSE;

	// get character font
	resources->characterFont = getBytes(file, 0xd908, 0xda19);

	// get strings
	resources->text_downland = getBytes(file, 0xda19, 0xda27);
	resources->text_writtenBy = getBytes(file, 0xda27, 0xda33);
	resources->text_michaelAichlmayer = getBytes(file, 0xda33, 0xda45);
	resources->text_copyright1983 = getBytes(file, 0xda45, 0xda54);
	resources->text_spectralAssociates = getBytes(file, 0xda54, 0xda68);
	resources->text_licensedTo = getBytes(file, 0xda68, 0xda75);
	resources->text_tandyCorporation = getBytes(file, 0xda75, 0xda87);
	resources->text_allRightsReserved = getBytes(file, 0xda87, 0xda9b);
	resources->text_onePlayer = getBytes(file, 0xda9b, 0xdaa6);
	resources->text_twoPlayer = getBytes(file, 0xdaa6, 0xdab1);
	resources->text_highScore = getBytes(file, 0xdab1, 0xdabc);
	resources->text_playerOne = getBytes(file, 0xdabc, 0xdac7);
	resources->text_playerTwo = getBytes(file, 0xdac7, 0xdad2);
	resources->text_pL1 = getBytes(file, 0xdad2, 0xdad6);
	resources->text_pL2 = getBytes(file, 0xdad6, 0xdada);
	resources->text_getReadyPlayerOne = getBytes(file, 0xdada, 0xdaef);
	resources->text_getReadyPlayerTwo = getBytes(file, 0xdaef, 0xdb04);
	resources->text_chamber = getBytes(file, 0xdb04, 0xdb0c);

	// get sprites
    resources->sprites_player = getBytes(file, 0xdcd6, 0xde17);
    resources->collisionmask_player = getBytes(file, 0xde17, 0xde7b);
    resources->sprites_bouncyBall = getBytes(file, 0xde7b, 0xde9b);
    resources->sprites_bird = getBytes(file, 0xde9b, 0xdeb3);
    resources->sprite_moneyBag = getBytes(file, 0xdeb3, 0xdec7);
    resources->sprite_diamond = getBytes(file, 0xdec7, 0xd3db);
    resources->sprite_key = getBytes(file, 0xd3db, 0xdeef);
    resources->sprite_playerSplat = getBytes(file, 0xdeef, 0xdf0a);
    resources->sprite_door = getBytes(file, 0xdf0a, 0xdf2a);
    resources->sprites_drops = getBytes(file, 0xdf2a, 0xdf5a);

	// get shapes data
	loadShapeDrawData(file, 0xd5f7, &resources->shapeDrawData_00_Stalactite);
	loadShapeDrawData(file, 0xd60c, &resources->shapeDrawData_01_WallGoingDown);
	loadShapeDrawData(file, 0xd616, &resources->shapeDrawData_07_WallPieceGoingUp);
	loadShapeDrawData(file, 0xd625, &resources->shapeDrawData_02_LeftHandCornerPiece);
	loadShapeDrawData(file, 0xd635, &resources->shapeDrawData_08_CornerPieceGoingDownLeft);
	loadShapeDrawData(file, 0xd644, &resources->shapeDrawData_03_TopRightHandCornerPiece);
	loadShapeDrawData(file, 0xd654, &resources->shapeDrawData_04_TopRightHandCornerPiece2);
	loadShapeDrawData(file, 0xd663, &resources->shapeDrawData_05_BottomRightSideOfFloatingPlatforms);
	loadShapeDrawData(file, 0xd67b, &resources->shapeDrawData_14_HorizontalRopeStartGoingRight);
	loadShapeDrawData(file, 0xd68d, &resources->shapeDrawData_15_HorizontalRopeEndGoingRight);
	loadShapeDrawData(file, 0xd697, &resources->shapeDrawData_17_BlankAreaGoingRight);
	loadShapeDrawData(file, 0xd6a0, &resources->shapeDrawData_18_BlankAreaGoingLeft);
	loadShapeDrawData(file, 0xd6a9, &resources->shapeDrawData_19_BlankAreaGoingDownRight);
	loadShapeDrawData(file, 0xd6b2, &resources->shapeDrawData_0b_ShortLineGoingUp);
	loadShapeDrawData(file, 0xd6d9, &resources->shapeDrawData_0c_VeryShortRope);
	loadShapeDrawData(file, 0xd6e5, &resources->shapeDrawData_0d_ShortRope);
	loadShapeDrawData(file, 0xd6f1, &resources->shapeDrawData_0e_MidLengthRope);
	loadShapeDrawData(file, 0xd6fd, &resources->shapeDrawData_0f_LongRope);
	loadShapeDrawData(file, 0xd709, &resources->shapeDrawData_10_VeryLongRope);
	loadShapeDrawData(file, 0xd715, &resources->shapeDrawData_11_SuperLongRope);
	loadShapeDrawData(file, 0xd721, &resources->shapeDrawData_12_ExcessivelyLongRope);
	loadShapeDrawData(file, 0xd72d, &resources->shapeDrawData_13_RediculouslyLongRope);
	loadShapeDrawData(file, 0xd74c, &resources->shapeDrawData_PreRope_Maybe);
	loadShapeDrawData(file, 0xd750, &resources->shapeDrawData_PostRope_Maybe);


	// get background resources
	loadBackgroundDrawData(file, 0xd35e, &resources->roomResources[0].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd3a0, &resources->roomResources[1].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd3e4, &resources->roomResources[2].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd426, &resources->roomResources[3].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd467, &resources->roomResources[4].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd4a2, &resources->roomResources[5].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd4d9, &resources->roomResources[6].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd4f6, &resources->roomResources[7].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd52f, &resources->roomResources[8].backgroundDrawData);
	loadBackgroundDrawData(file, 0xd561, &resources->roomResources[9].backgroundDrawData);
	loadBackgroundDrawData(file, 0xcec4, &resources->roomResources[10].backgroundDrawData);

	loadDropSpawnPositions(file, 0xd073, &resources->roomResources[0].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd089, &resources->roomResources[1].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd09c, &resources->roomResources[2].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd0ac, &resources->roomResources[3].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd0bf, &resources->roomResources[4].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd0cf, &resources->roomResources[5].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd0df, &resources->roomResources[6].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd0e6, &resources->roomResources[7].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd0fc, &resources->roomResources[8].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd10c, &resources->roomResources[9].dropSpawnPositions);
	loadDropSpawnPositions(file, 0xd116, &resources->roomResources[10].dropSpawnPositions);

	fclose(file);

	return TRUE;
}

void ResourceLoader_Shutdown(Resources* resources)
{
	free(resources->characterFont);

	free(resources->text_downland);
	free(resources->text_writtenBy);
	free(resources->text_michaelAichlmayer);
	free(resources->text_copyright1983);
	free(resources->text_spectralAssociates);
	free(resources->text_licensedTo);
	free(resources->text_tandyCorporation);
	free(resources->text_allRightsReserved);
	free(resources->text_onePlayer);
	free(resources->text_twoPlayer);
	free(resources->text_highScore);
	free(resources->text_playerOne);
	free(resources->text_playerTwo);
	free(resources->text_pL1);
	free(resources->text_pL2);
	free(resources->text_getReadyPlayerOne);
	free(resources->text_getReadyPlayerTwo);
	free(resources->text_chamber);

	// get sprites
    free(resources->sprites_player);
    free(resources->collisionmask_player);
    free(resources->sprites_bouncyBall);
    free(resources->sprites_bird);
    free(resources->sprite_moneyBag);
    free(resources->sprite_diamond);
    free(resources->sprite_key);
    free(resources->sprite_playerSplat);
    free(resources->sprite_door);
    free(resources->sprites_drops);

	// free shapes data
	free(resources->shapeDrawData_00_Stalactite.segments);
	free(resources->shapeDrawData_01_WallGoingDown.segments);
	free(resources->shapeDrawData_07_WallPieceGoingUp.segments);
	free(resources->shapeDrawData_02_LeftHandCornerPiece.segments);
	free(resources->shapeDrawData_08_CornerPieceGoingDownLeft.segments);
	free(resources->shapeDrawData_03_TopRightHandCornerPiece.segments);
	free(resources->shapeDrawData_04_TopRightHandCornerPiece2.segments);
	free(resources->shapeDrawData_05_BottomRightSideOfFloatingPlatforms.segments);
	free(resources->shapeDrawData_14_HorizontalRopeStartGoingRight.segments);
	free(resources->shapeDrawData_15_HorizontalRopeEndGoingRight.segments);
	free(resources->shapeDrawData_17_BlankAreaGoingRight.segments);
	free(resources->shapeDrawData_18_BlankAreaGoingLeft.segments);
	free(resources->shapeDrawData_19_BlankAreaGoingDownRight.segments);
	free(resources->shapeDrawData_0b_ShortLineGoingUp.segments);
	free(resources->shapeDrawData_0c_VeryShortRope.segments);
	free(resources->shapeDrawData_0d_ShortRope.segments);
	free(resources->shapeDrawData_0e_MidLengthRope.segments);
	free(resources->shapeDrawData_0f_LongRope.segments);
	free(resources->shapeDrawData_10_VeryLongRope.segments);
	free(resources->shapeDrawData_11_SuperLongRope.segments);
	free(resources->shapeDrawData_12_ExcessivelyLongRope.segments);
	free(resources->shapeDrawData_13_RediculouslyLongRope.segments);
	free(resources->shapeDrawData_PreRope_Maybe.segments);
	free(resources->shapeDrawData_PostRope_Maybe.segments);

	// free background resources
	for (int loop = 0; loop < NUM_ROOMS; loop++)
	{
		free(resources->roomResources[loop].backgroundDrawData.backgroundDrawCommands);
		free(resources->roomResources[loop].dropSpawnPositions.dropSpawnAreas);
	}
}