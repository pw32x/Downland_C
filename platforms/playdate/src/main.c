#include "pd_api.h"

#include "..\..\..\game\dl_sound.h"
#include "..\..\..\game\base_types.h"
#include "..\..\..\game\resource_types.h"
#include "..\..\..\game\resource_loader_buffer.h"
#include "..\..\..\game\checksum_utils.h"
#include "..\..\..\game\game.h"

PlaydateAPI* g_pd = NULL;

static bool init(PlaydateAPI* pd);
static int update(void* userdata);

GameData gameData;
Resources resources;

AudioSample* g_sounds[SOUND_NUM_SOUNDS];

SamplePlayer* g_samplePlayers[SOUND_NUM_SOUNDS];


void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	if (loop && g_pd->sound->sampleplayer->isPlaying(g_samplePlayers[soundIndex]))
		return;

	g_pd->sound->sampleplayer->setPaused(g_samplePlayers[soundIndex], false);
	g_pd->sound->sampleplayer->play(g_samplePlayers[soundIndex], !loop, 1.0f);
}

void Sound_Stop(dl_u8 soundIndex)
{
	g_pd->sound->sampleplayer->stop(g_samplePlayers[soundIndex]);
}

void Sound_PauseAll(dl_u8 pause)
{
	for (int loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
	{
		g_pd->sound->sampleplayer->setPaused(g_samplePlayers[loop], pause);
	}
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	switch (event)
	{
	case kEventInit:
	{
		if (!init(pd))
		{
			return -1;
		}

		// Note: If you set an update callback in the kEventInit handler, the system assumes the 
		// game is pure C and doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
		break;
	}
	case kEventLock:
	case kEventPause:
	{
		gameData.paused = true;
		Sound_PauseAll(gameData.paused);
		break;
	}
	case kEventUnlock:
	case kEventResume:
	{
		gameData.paused = false;
		Sound_PauseAll(gameData.paused);
		break;
	}

	}
	
	return 0;
}

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

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{
    memset(source, value, count);
}

void dl_memcpy(void* destination, const void* source, dl_u16 count)
{
    memcpy(destination, source, count);
}

#define FILE_BUFFER_SIZE 8192
dl_u8 fileBuffer[FILE_BUFFER_SIZE];

const char* romFileNames[] = 
{
    "downland.bin",
    "downland.rom",
    "Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc"
};

int romFileNamesCount = sizeof(romFileNames) / sizeof(romFileNames[0]);


static bool loadFile(const char* romPath, dl_u8* fileBuffer, dl_u32 fileBufferSize)
{
	SDFile* file = g_pd->file->open(romPath, kFileRead);

	if (file == NULL)
		return false;

    g_pd->file->seek(file, 0L, SEEK_END);
    dl_u32 fileSize = g_pd->file->tell(file);

    if (fileSize != fileBufferSize)
    {
        g_pd->file->close(file);
        return false;
    }

    g_pd->file->seek(file, 0L, SEEK_SET);

    int bytesRead = (int)g_pd->file->read(file, fileBuffer, fileBufferSize);

    if (bytesRead != fileBufferSize)
    {
        g_pd->file->close(file);
        return false;
    }

    g_pd->file->close(file);
	return true;
}

bool init(PlaydateAPI* pd)
{
	g_pd = pd;

    bool romFoundAndLoaded = false;
    for (int loop = 0; loop < romFileNamesCount; loop++)
    {
        if (loadFile(romFileNames[loop], fileBuffer, FILE_BUFFER_SIZE) &&
            checksumCheckLitteEndian(fileBuffer, FILE_BUFFER_SIZE) &&
            ResourceLoaderBuffer_Init(fileBuffer, FILE_BUFFER_SIZE, &resources))
        {
            romFoundAndLoaded = true;
            break;
        }
    }

    if (!romFoundAndLoaded)
    {
        return false;
    }

	g_sounds[SOUND_JUMP] = g_pd->sound->sample->load("jump.wav");
	g_sounds[SOUND_LAND] = g_pd->sound->sample->load("land.wav");
	g_sounds[SOUND_SCREEN_TRANSITION] = g_pd->sound->sample->load("transition.wav");
	g_sounds[SOUND_SPLAT] = g_pd->sound->sample->load("splat.wav");
	g_sounds[SOUND_PICKUP] = g_pd->sound->sample->load("pickup.wav");
	g_sounds[SOUND_RUN] = g_pd->sound->sample->load("run.wav");
	g_sounds[SOUND_CLIMB_UP] = g_pd->sound->sample->load("climb_up.wav");
	g_sounds[SOUND_CLIMB_DOWN] = g_pd->sound->sample->load("climb_down.wav");

	g_samplePlayers[SOUND_JUMP] = g_pd->sound->sampleplayer->newPlayer();
	g_samplePlayers[SOUND_LAND] = g_pd->sound->sampleplayer->newPlayer();
	g_samplePlayers[SOUND_SCREEN_TRANSITION] = g_pd->sound->sampleplayer->newPlayer();
	g_samplePlayers[SOUND_SPLAT] = g_pd->sound->sampleplayer->newPlayer();
	g_samplePlayers[SOUND_PICKUP] = g_pd->sound->sampleplayer->newPlayer();
	g_samplePlayers[SOUND_RUN] = g_pd->sound->sampleplayer->newPlayer();
	g_samplePlayers[SOUND_CLIMB_UP] = g_pd->sound->sampleplayer->newPlayer();
	g_samplePlayers[SOUND_CLIMB_DOWN] = g_pd->sound->sampleplayer->newPlayer();

	for (int loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
		pd->sound->sampleplayer->setSample(g_samplePlayers[loop], g_sounds[loop]);

	memset(&gameData, 0, sizeof(GameData));
	Game_Init(&gameData, &resources);

	return true;
}


void updateControls(JoystickState* joystickState)
{
	PDButtons buttonState;

	g_pd->system->getButtonState(&buttonState, NULL, NULL);

    // Check D-Pad
    bool leftDown = buttonState & kButtonLeft;
    bool rightDown = buttonState & kButtonRight;
    bool upDown = buttonState & kButtonUp;
    bool downDown = buttonState & kButtonDown;
    bool jumpDown = buttonState & kButtonA;
    //bool startDown = buttonState & KEY_START;

    joystickState->leftPressed = !joystickState->leftDown & leftDown;
    joystickState->rightPressed = !joystickState->rightDown & rightDown;
    joystickState->upPressed = !joystickState->upDown & upDown;
    joystickState->downPressed =  !joystickState->downDown & downDown;
    joystickState->jumpPressed =  !joystickState->jumpDown & jumpDown;
    //joystickState->startPressed = !joystickState->startDown & startDown;

    joystickState->leftReleased = joystickState->leftDown & !leftDown;
    joystickState->rightReleased = joystickState->rightDown & !rightDown;
    joystickState->upReleased = joystickState->upDown & !upDown;
    joystickState->downReleased =  joystickState->downDown & !downDown;
    joystickState->jumpReleased =  joystickState->jumpDown & !jumpDown;
    //joystickState->startReleased = joystickState->startPressed & !startDown;

    joystickState->leftDown = leftDown;
    joystickState->rightDown = rightDown;
    joystickState->upDown = upDown;
    joystickState->downDown = downDown;
    joystickState->jumpDown = jumpDown;
    //joystickState->startDown = startDown;

#ifdef DEV_MODE
    bool debugStateDown = buttonState & kButtonB;

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
}

static int update(void* userdata)
{
	if (!gameData.paused)
	{
		updateControls(&gameData.joystickState);

		// update game twice because the original
		// game runs at 60fps
		Game_Update(&gameData, &resources);
		Game_Update(&gameData, &resources);

		uint8_t* backbuffer = g_pd->graphics->getFrame(); // working buffer

		g_pd->graphics->clear(kColorBlack);

		for (int loop = 0; loop < FRAMEBUFFER_HEIGHT; loop++)
		{
			memcpy(backbuffer + (LCD_ROWSIZE * (loop + 25)) + 9, 
				   gameData.framebuffer + (FRAMEBUFFER_PITCH * loop),
				   FRAMEBUFFER_PITCH); 
		}

		g_pd->graphics->markUpdatedRows(0, LCD_ROWS);
	}

	return 1;
}

