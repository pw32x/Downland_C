#include "resources.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
	byte* segmentsMemory = (byte*)malloc(bufferSize);

	if (segmentsMemory == NULL)
		return;

	fread(segmentsMemory, bufferSize, 1, file);

	shapeDrawData->segments = (ShapeSegment*)segmentsMemory;
}

BOOL Resources_Init(const char* romPath, Resources* resources)
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

	// title screen background
	loadBackgroundDrawData(file, 0xcec4, &resources->backgroundDrawData_TitleScreen);

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

	fclose(file);

	return TRUE;
}

void Resources_Shutdown(Resources* resources)
{
	free(resources->characterFont);

	// free background draw data
	free(resources->backgroundDrawData_TitleScreen.backgroundDrawCommands);

	// free draw data
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
}