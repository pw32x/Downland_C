#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

#include "..\..\..\game\dl_sound.h"
#include "..\..\..\game\base_types.h"
#include "..\..\..\game\resource_types.h"
#include "..\..\..\game\resource_loader_buffer.h"
#include "..\..\..\game\checksum_utils.h"
#include "..\..\..\game\game.h"

#include "game_runner.h"

PlaydateAPI* g_pd = NULL;

static bool init(PlaydateAPI* pd);
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
		if (!init(pd))
		{
			return -1;
		}

		const char* err;
		font = pd->graphics->loadFont(fontpath, &err);
		
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		// Note: If you set an update callback in the kEventInit handler, the system assumes the 
		// game is pure C and doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);


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

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}


#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

int x = (400-TEXT_WIDTH)/2;
int y = (240-TEXT_HEIGHT)/2;
int dx = 1;
int dy = 2;

GameData gameData;
Resources resources;

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

	memset(&gameData, 0, sizeof(GameData));
	GameRunner_Init(&gameData, &resources);

	
	// half-ass'd Downland to Ascii string conversion
	dl_u8* runner = resources.text_michaelAichlmayer;

	while (*runner != 0xff)
	{
		*runner = *runner + 55;

		runner++;
	}

	return true;
}

static int update(void* userdata)
{
	GameRunner_Update(&gameData, &resources);
	GameRunner_Draw(&gameData, &resources);

	g_pd->graphics->clear(kColorWhite);
	g_pd->graphics->setFont(font);	

	g_pd->graphics->drawText(resources.text_michaelAichlmayer, strlen("Hello World!"), kASCIIEncoding, x, y);

	x += dx;
	y += dy;
	
	if ( x < 0 || x > LCD_COLUMNS - TEXT_WIDTH )
		dx = -dx;
	
	if ( y < 0 || y > LCD_ROWS - TEXT_HEIGHT )
		dy = -dy;
        
	g_pd->system->drawFPS(0,0);

	return 1;
}

