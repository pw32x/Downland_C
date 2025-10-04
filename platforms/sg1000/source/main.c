#include <stdio.h>
#include "SGlib.h"
#include <PSGLib.h>

#include "sounds.h"

#define TRUE 1
#define FALSE 0

#define SOUND_JUMP				0
#define SOUND_LAND				1
#define SOUND_SCREEN_TRANSITION	2
#define SOUND_SPLAT				3
#define SOUND_PICKUP			4
#define SOUND_RUN				5
#define SOUND_CLIMB_UP			6
#define SOUND_CLIMB_DOWN		7
#define SOUND_NUM_SOUNDS		8

typedef unsigned char dl_u8;

void PSGUpdate(void)
{
	PSGSFXFrame();
}

const dl_u8* sounds[SOUND_NUM_SOUNDS] = 
{
	jump_psg, // SOUND_JUMP				
	land_psg, // SOUND_LAND				
	transition_psg, // SOUND_SCREEN_TRANSITION	
	splat_psg, // SOUND_SPLAT				
	pickup_psg, // SOUND_PICKUP			
	run_psg, // SOUND_RUN				
	climb_up_psg, // SOUND_CLIMB_UP			
	climb_down_psg, // SOUND_CLIMB_DOWN		
};


dl_u8 isPlaying[SOUND_NUM_SOUNDS];

const dl_u8 isLooped[SOUND_NUM_SOUNDS] = 
{
	FALSE, // SOUND_JUMP				
	FALSE, // SOUND_LAND				
	FALSE, // SOUND_SCREEN_TRANSITION	
	FALSE, // SOUND_SPLAT				
	FALSE, // SOUND_PICKUP			
	TRUE, // SOUND_RUN				
	TRUE, // SOUND_CLIMB_UP			
	TRUE, // SOUND_CLIMB_DOWN		
};

const dl_u8 channels[SOUND_NUM_SOUNDS] = 
{
	SFX_CHANNEL2,
	SFX_CHANNELS2AND3,
	SFX_CHANNEL2,
	SFX_CHANNELS2AND3,
	SFX_CHANNEL2,
	SFX_CHANNELS2AND3,
	SFX_CHANNELS2AND3,
	SFX_CHANNELS2AND3,
};

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	if (loop)
	{
		if (isPlaying[soundIndex])
			return;

		PSGSFXPlayLoop(sounds[soundIndex], channels[soundIndex]);
		isPlaying[soundIndex] = TRUE;
	}
	else
		PSGSFXPlay(sounds[soundIndex], channels[soundIndex]);
}

void Sound_Stop(dl_u8 soundIndex)
{
	if (isLooped[soundIndex] && isPlaying[soundIndex])
		PSGSFXStop();

	isPlaying[soundIndex] = FALSE;
}

void Sound_StopAll(void)
{
}


void main(void)
{
	SG_VRAMmemset(0, 0x0000, 16384);

	SG_displayOn();
	SG_setBackdropColor(SG_COLOR_LIGHT_BLUE);

	SG_setFrameInterruptHandler(PSGUpdate);


	Sound_Play(SOUND_JUMP, FALSE);

	for(;;) 
	{ 
		// Game Loop
		SG_initSprites();

		// VBLANK
		SG_waitForVBlank();

		SG_copySpritestoSAT();
	}
}
