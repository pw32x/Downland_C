#ifndef DOOR_UTILS_INCLUDE_H
#define DOOR_UTILS_INCLUDE_H

#include "base_types.h"
#include "door_types.h"

void drawDoor(DoorInfo* doorInfo, 
			  u8* bitShiftedDoorSprites, 
			  u8* framebuffer, 
			  u8* cleanBackground,
			  u8 drawOnFramebuffer); // we draw on framebuffer when the door gets activated
									 // but not during a room transition

#endif
