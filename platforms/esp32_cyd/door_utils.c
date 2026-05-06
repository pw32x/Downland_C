#include "door_utils.h"
#include "draw_utils.h"

void drawDoor(const DoorInfo* doorInfo, 
			  const dl_u8* bitShiftedDoorSprites, 
			  dl_u8* framebuffer, 
			  dl_u8* cleanBackground,
			  dl_u8 drawOnFramebuffer)
{
	const dl_u8* doorSprite;

	// draw the door. 
	dl_u8 y = doorInfo->y;
	dl_u8 x = doorInfo->x;

	// adjust the door position, as per the original game.
	if (x > 40) 
		x += 7;
	else
		x -= 4;

	doorSprite = getBitShiftedSprite(bitShiftedDoorSprites, 
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

