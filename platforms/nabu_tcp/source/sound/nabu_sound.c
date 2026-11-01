#include <arch/nabu.h>

#include "nabu_sound.h"
#include "sound_data.h"

extern const unsigned char* psg_ptr;    // in psgdac.c
extern const unsigned char* psg_next;
extern unsigned char psg_done;
extern unsigned char psg_mixer_kept;
extern void psg_start_frame(void);

static const unsigned char* oneShotPos = 0;   // the next frame of the effect that is playing once
static const unsigned char* loopPos = 0;      // the next frame of the looping effect
static signed char loopSound = -1;

static void ay_write(unsigned char reg, unsigned char value)
{
	IO_AYLATCH = reg;
	IO_AYDATA = value;
}

static void silence(void)
{
	ay_write(7, psg_mixer_kept | 0x3F);	// no tone, no noise (and the I/O ports as the system left them)
	ay_write(8, 0);						// all three channels off
	ay_write(9, 0);
	ay_write(10, 0);
	IO_AYLATCH = 14;					// leave the interrupt mask register selected
}

void nabusound_init(void)
{
	// the I/O port direction bits of the mixer register, which the sound keeps as they are
	IO_AYLATCH = 7;
	psg_mixer_kept = IO_AYDATA & 0xC0;
	silence();
}

void nabusound_frame(void)
{
	if (oneShotPos)
	{
		psg_ptr = oneShotPos;
		psg_start_frame();
		if (psg_done)
		{
			oneShotPos = 0;
		}
		else
		{
			oneShotPos = psg_next;
		}
	}
	else if (loopSound >= 0)
	{
		psg_ptr = loopPos;
		psg_start_frame();
		if (psg_done)
			loopPos = sound_streams[(int)loopSound];   // start again
		else
			loopPos = psg_next;
	}
}

void nabusound_playLoop(unsigned char soundIndex)
{
	if (loopSound == (signed char)soundIndex)
		return;

	loopSound = soundIndex;
	loopPos = sound_streams[soundIndex];
}

void nabusound_play(unsigned char soundIndex)
{
	silence();
	oneShotPos = sound_streams[soundIndex];
}

void nabusound_stopLoop(unsigned char soundIndex)
{
	if (loopSound == (signed char)soundIndex)
	{
		loopSound = -1;
		loopPos = 0;
		silence();
	}
}

void nabusound_stopAll(void)
{
	oneShotPos = 0;
	loopSound = -1;
	loopPos = 0;
	silence();
}
