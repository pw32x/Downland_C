#ifndef BASE_DEFINES_INCLUDE_H
#define BASE_DEFINES_INCLUDE_H

#define FRAMEBUFFER_WIDTH  256
#define FRAMEBUFFER_HEIGHT 192
#define FRAMEBUFFER_PITCH  (FRAMEBUFFER_WIDTH / 8)
#define FRAMEBUFFER_SIZE_IN_BYTES   0x1800

#define GET_FRAMEBUFFER_LOCATION(x, y) ((x / 4) + (y * FRAMEBUFFER_PITCH))

#define NUM_ROOMS 10
#define NUM_ROOMS_PLUS_TITLESCREN 11
#define NUM_ROOMS_AND_ALL 12
#define TITLE_SCREEN_ROOM_INDEX 10	
#define TRANSITION_ROOM_INDEX 11

#define PLAYERLIVES_ICON_X	0x14
#define PLAYERLIVES_ICON_Y	0x2
#define PLAYERLIVES_DRAW_LOCATION  GET_FRAMEBUFFER_LOCATION(PLAYERLIVES_ICON_X, PLAYERLIVES_ICON_Y)
#define PLAYERLIVES_TEXT_DRAW_LOCATION  (PLAYERLIVES_DRAW_LOCATION - 0x4)
#define CHAMBER_TEXT_DRAW_LOCATION  0x55

#define PLAYERLIVES_ICON_SPACING 0x3
#define PLAYERLIVES_ICON_LINESTODRAW 0x7

#endif