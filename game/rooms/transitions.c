#include "room_types.h"

#include <string.h>

#include "../game_types.h"
#include "../draw_utils.h"
#include "../drops_manager.h"
#include "../dl_sound.h"

#define INITIAL_TRANSITION_DELAY 30

// This simulates the pause before a room appears in the original game.
// In the original game, the pause is because the background is being
// drawn in the clearBackground off-screen buffer.
void transition_init(Room* targetRoom, GameData* gameData, const Resources* resources)
{
	// init the clean background with the target room. 
	// it'll be revealed at the end of the transition.
	targetRoom->draw(gameData->transitionRoomNumber, (struct GameData*)gameData, resources);

	// setup screen transition
	gameData->transitionInitialDelay = INITIAL_TRANSITION_DELAY;
	memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);
}

void transition_update(Room* room, GameData* gameData, const Resources* resources)
{
	// wait to draw anything until the delay is over
	if (gameData->transitionInitialDelay)
	{
		gameData->transitionInitialDelay--;
		return;
	}

	// dump the cleanBackground to the framebuffer and go 
	// to the next room.
	memcpy(gameData->framebuffer, gameData->cleanBackground, FRAMEBUFFER_SIZE_IN_BYTES);

	Game_EnterRoom(gameData, gameData->transitionRoomNumber, resources);
}

Room transitionRoom =
{
	TRANSITION_ROOM_INDEX,
	(InitRoomFunctionType)transition_init,
	NULL, // don't draw anything. 
	(UpdateRoomFunctionType)transition_update
};

void wipe_transition_init(Room* targetRoom, GameData* gameData, const Resources* resources)
{
	// init the clean background with the target room. 
	// we'll be slowly revealing it during the room transition.
	targetRoom->draw(gameData->transitionRoomNumber, (struct GameData*)gameData, resources);

	// setup screen transition
	gameData->transitionInitialDelay = INITIAL_TRANSITION_DELAY;
	gameData->transitionCurrentLine = 0;
	gameData->transitionFrameDelay = 0;
}

void wipe_transition_update(Room* room, GameData* gameData, const Resources* resources)
{
	dl_u8 loopCount;
	dl_u8 loopCounter;
	dl_u16 offset;
	dl_u8* cleanBackgroundRunner;
	dl_u8* framebufferRunner;
	int loop;
	int innerLoop;

	if (gameData->transitionInitialDelay)
	{
		gameData->transitionInitialDelay--;
		return;
	}

	// only update every other frame to simulate the 
	// original game.
	gameData->transitionFrameDelay = !gameData->transitionFrameDelay;

	if (gameData->transitionFrameDelay)
		return;

	// play the transition sound effect at the start
	if (!gameData->transitionCurrentLine)
	{
		Sound_Play(SOUND_SCREEN_TRANSITION, FALSE);
	}

	// do two lines at every four so that we
	// can finish when the transition sound effect does.
	loopCount = gameData->transitionCurrentLine % 4 == 0 ? 2 : 1;

	for (loopCounter = 0; loopCounter < loopCount; loopCounter++)
	{
		offset = gameData->transitionCurrentLine * FRAMEBUFFER_PITCH;

		cleanBackgroundRunner = gameData->cleanBackground + offset;
		framebufferRunner = gameData->framebuffer + offset;

		// the screen is divided in six horizontal strips. Every frame,
		// a horizontal line of every strip is revealed, copied from the
		// cleanBuffer to the framebuffer.
		for (loop = 0; loop < 6; loop++)
		{
			for (innerLoop = 0; innerLoop < FRAMEBUFFER_PITCH; innerLoop++)
			{
				*framebufferRunner = *cleanBackgroundRunner;

				// draw a dotted line underneath the pixel, but not
				// for the last line.
				if (gameData->transitionCurrentLine < 31)
				{
					*(framebufferRunner + FRAMEBUFFER_PITCH) = CRT_EFFECT_MASK;
				}

				framebufferRunner++;
				cleanBackgroundRunner++;
			}

			// move to the next strip, 31 * 256 pixels down.
			// which is 992 bytes over because 1 byte is 8 pixels.
			// it's not 32 pixels down because we're already at the
			// end of the line for this strip.
			cleanBackgroundRunner += 0x3e0; 
			framebufferRunner += 0x3e0;		
		}

		gameData->transitionCurrentLine++;
	}

	// we're done
	if (gameData->transitionCurrentLine == 32)
	{
		Game_EnterRoom(gameData, gameData->transitionRoomNumber, resources);

		if (Game_TransitionDone)
			Game_TransitionDone(gameData, gameData->transitionRoomNumber, WIPE_TRANSITION_ROOM_INDEX);
	}
}

Room wipeTransitionRoom =
{
	WIPE_TRANSITION_ROOM_INDEX,
	(InitRoomFunctionType)wipe_transition_init,
	NULL,
	(UpdateRoomFunctionType)wipe_transition_update
};
