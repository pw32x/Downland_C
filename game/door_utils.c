#include "door_utils.h"
#include "draw_utils.h"

void drawDoor(const DoorInfo* doorInfo, 
			  const u8* bitShiftedDoorSprites, 
			  u8* framebuffer, 
			  u8* cleanBackground,
			  u8 drawOnFramebuffer)
{
	// draw the door. 
	u8 y = doorInfo->y;
	u8 x = doorInfo->x;

	// adjust the door position, as per the original game.
	if (x > 40) 
		x += 7;
	else
		x -= 4;

	const u8* doorSprite = getBitShiftedSprite(bitShiftedDoorSprites, 
											   0,
											   x & 3, 
											   DOOR_BITSHIFTED_SPRITE_FRAME_SIZE);

	if (drawOnFramebuffer)
	{
		drawSprite_24PixelsWide(doorSprite, 
								x, 
								y, 
								DOOR_SPRITE_ROWS, 
								framebuffer);
	}

	drawSprite_24PixelsWide(doorSprite, 
							x, 
							y, 
							DOOR_SPRITE_ROWS, 
							cleanBackground);

}

