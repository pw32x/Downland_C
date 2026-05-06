#ifndef DOOR_UTILS_INCLUDE_H
#define DOOR_UTILS_INCLUDE_H

#include "base_types.h"
#include "door_types.h"

void drawDoor(const DoorInfo* doorInfo, 
			  const dl_u8* bitShiftedDoorSprites, 
			  dl_u8* framebuffer, 
			  dl_u8* cleanBackground,
			  dl_u8 drawOnFramebuffer); // we draw on framebuffer when the door gets activated
									 // but not during a room transition

#endif
