#include "room_types.h"

#include <string.h>

#include "..\game_types.h"
#include "..\draw_utils.h"
#include "..\drops_manager.h"
#include "..\dl_sound.h"

// This simulates the pause before a room appears in the original game.
// In the original game, the pause is because the background is being
// drawn in the clearBackground off-screen buffer.
void transition_init(Room* targetRoom, GameData* gameData, Resources* resources)
{
	// init the clean background with the target room. 
	// it'll be revealed at the end of the transition.
	targetRoom->draw(gameData->transitionRoomNumber, (struct GameData*)gameData, resources);

	// setup screen transition
	gameData->transitionInitialDelay = 30;
	memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);
}

void transition_update(Room* room, GameData* gameData, Resources* resources)
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

void wipe_transition_init(Room* targetRoom, GameData* gameData, Resources* resources)
{
	// init the clean background with the target room. 
	// we'll be slowly revealing it during the room transition.
	targetRoom->draw(gameData->transitionRoomNumber, (struct GameData*)gameData, resources);

	// setup screen transition
	gameData->transitionInitialDelay = 30;
	gameData->transitionCurrentLine = 0;
	gameData->transitionFrameDelay = 0;
}

void wipe_transition_update(Room* room, GameData* gameData, Resources* resources)
{
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
	u8 loopCount = gameData->transitionCurrentLine % 4 == 0 ? 2 : 1;

	for (u8 loopCounter = 0; loopCounter < loopCount; loopCounter++)
	{
		u16 offset = gameData->transitionCurrentLine * FRAMEBUFFER_PITCH;

		u8* cleanBackgroundRunner = gameData->cleanBackground + offset;
		u8* framebufferRunner = gameData->framebuffer + offset;

		// the screen is divided in six horizontal strips. Every frame,
		// a horizontal line of every strip is revealed, copied from the
		// cleanBuffer to the framebuffer.
		for (int loop = 0; loop < 6; loop++)
		{
			for (int innerLoop = 0; innerLoop < FRAMEBUFFER_PITCH; innerLoop++)
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
