//
//  main.c
//  Extension
//
//  Created by Dave Hayden on 7/30/14.
//  Copyright (c) 2014 Panic, Inc. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

#include "..\..\..\game\dl_sound.h"

#include "..\..\..\game\base_types.h"
#include "..\..\..\game\resource_types.h"
#include "..\..\..\game\resource_loader_buffer.h"
#include "..\..\..\game\checksum_utils.h"
#include "..\..\..\game\game.h"

static dl_u8 memory[18288];
static dl_u8* memoryEnd = NULL;
void* dl_alloc(dl_u32 size)
{
	if (memoryEnd == NULL)
	{
		memoryEnd = memory;
	}

	dl_u8* memory = memoryEnd;

	memoryEnd += size;

	return (void*)memory;
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

static int update(void* userdata);
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		const char* err;
		font = pd->graphics->loadFont(fontpath, &err);
		
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		// Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
	}
	
	return 0;
}


#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

int x = (400-TEXT_WIDTH)/2;
int y = (240-TEXT_HEIGHT)/2;
int dx = 1;
int dy = 2;

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);
	pd->graphics->drawText("Hello World!", strlen("Hello World!"), kASCIIEncoding, x, y);

	x += dx;
	y += dy;
	
	if ( x < 0 || x > LCD_COLUMNS - TEXT_WIDTH )
		dx = -dx;
	
	if ( y < 0 || y > LCD_ROWS - TEXT_HEIGHT )
		dy = -dy;
        
	pd->system->drawFPS(0,0);

	return 1;
}

