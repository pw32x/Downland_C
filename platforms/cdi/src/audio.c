#include <strings.h>
#include <csd.h>
#include <sysio.h>
#include <cdfm.h>
#include <stdio.h>
#include <memory.h>
#include "audio.h"
#include "sfx_clips.h"

// One CD-XA sector's worth of audio, and the "chunk" unit sm_creat() counts
// buffers in (128 bytes = one sound group; 18 of them make one frame). These
// match the layout tools/sfx/build_sfx.py packs into SFX.BIN.
#define CHUNK_SIZE 128
#define CHUNKS_PER_FRAME 18
#define FRAME_SIZE (CHUNK_SIZE * CHUNKS_PER_FRAME)

// Marks a sound whose sound map could not be created, and the state where no sound is playing.
#define NO_SOUND_MAP -1
#define NO_SOUND_PLAYING -1

// How many times sd_loop() repeats a looping sound before it stops on its own.
#define SOUND_LOOP_REPEAT_COUNT 0x0FFF

// sc_atten() value for no attenuation on any channel, which is full volume.
#define FULL_VOLUME_ATTENUATION 0x00000000

dl_s32 audioDevicePath;

// The whole of SFX.BIN, loaded from disc. Each sound is copied out of here into its own sound map.
static dl_u8* sfxBinData;

// The frame in SFX.BIN where each sound starts
static dl_u16 soundStartFrame[SOUND_NUM_SOUNDS] =
{
	CLIP_JUMP_START,
	CLIP_LAND_START,
	CLIP_TRANSITION_START,
	CLIP_SPLAT_START,
	CLIP_PICKUP_START,
	CLIP_RUN_START,
	CLIP_CLIMB_UP_START,
	CLIP_CLIMB_DOWN_START
};

// How many frames long each sound is
static dl_u16 soundFrameCount[SOUND_NUM_SOUNDS] =
{
	CLIP_JUMP_FRAMES,
	CLIP_LAND_FRAMES,
	CLIP_TRANSITION_FRAMES,
	CLIP_SPLAT_FRAMES,
	CLIP_PICKUP_FRAMES,
	CLIP_RUN_FRAMES,
	CLIP_CLIMB_UP_FRAMES,
	CLIP_CLIMB_DOWN_FRAMES
};

// The audio driver's handle for each sound's sound map, from sm_creat()
static dl_s32 soundMapIds[SOUND_NUM_SOUNDS];

static STAT_BLK playbackStatus;

static dl_s8 currentPlayingSoundIndex = NO_SOUND_PLAYING;

static void createSoundMaps()
{
	dl_u8 loop;
	char* soundMapBuffer;

	for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
	{
		soundMapIds[loop] = sm_creat(audioDevicePath, D_CSTRO, soundFrameCount[loop] * CHUNKS_PER_FRAME, &soundMapBuffer);

		if (soundMapIds[loop] != NO_SOUND_MAP)
		{
			memcpy(soundMapBuffer, sfxBinData + soundStartFrame[loop] * FRAME_SIZE, soundFrameCount[loop] * FRAME_SIZE);
		}
	}
}

static void initSoundEffects()
{
	dl_u8 loop;

	for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
	{
		soundMapIds[loop] = NO_SOUND_MAP;
	}

	sfxBinData = (dl_u8*)srqcmem(SFX_SIZE, SYSRAM);
}

dl_s32 loadSoundEffects(dl_s32 file)
{
	dl_s32 bytesRead = read(file, (char*)sfxBinData, SFX_SIZE);

	if (bytesRead > 0)
	{
		createSoundMaps();
	}

	return bytesRead;
}

dl_u8 Sound_IsPlaying(dl_u8 soundIndex)
{
	return (currentPlayingSoundIndex == soundIndex);
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	if (soundIndex >= SOUND_NUM_SOUNDS) 
	{
		return;
	}

	if (soundMapIds[soundIndex] == NO_SOUND_MAP) 
	{
		return;
	}

	if (loop)
	{
		dl_u16 lastChunk;

		// ignore if we're already looping
		if (Sound_IsPlaying(soundIndex))
		{
			return;
		}

		// The hardware repeats the sound map by itself. The loop range is
		// in chunks, so it runs from the first chunk to the last chunk of
		// this sound, covering the whole sound.
		lastChunk = soundFrameCount[soundIndex] * CHUNKS_PER_FRAME - 1;
		sd_loop(audioDevicePath, soundMapIds[soundIndex], 0, lastChunk, SOUND_LOOP_REPEAT_COUNT);
	}

	sm_out(audioDevicePath, soundMapIds[soundIndex], &playbackStatus);
	currentPlayingSoundIndex = soundIndex;
}

void Sound_Stop(dl_u8 soundIndex)
{
	if (soundIndex >= SOUND_NUM_SOUNDS) 
	{
		return;
	}

	if (currentPlayingSoundIndex != soundIndex) 
	{
		return;
	}

	sm_off(audioDevicePath);
	currentPlayingSoundIndex = NO_SOUND_PLAYING;
}

void initAudio(void)
{
	char* deviceName = csd_devname(DT_AUDIO, 1);
	audioDevicePath = open(deviceName, UPDAT_);
	free(deviceName);

	sc_atten(audioDevicePath, FULL_VOLUME_ATTENUATION);

	initSoundEffects();
}

void closeAudio(void)
{
	dl_u8 loop;

	sm_off(audioDevicePath);

	for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
	{
		if (soundMapIds[loop] != NO_SOUND_MAP)
		{
			sm_close(audioDevicePath, soundMapIds[loop]);
		}
	}

	close(audioDevicePath);
}
