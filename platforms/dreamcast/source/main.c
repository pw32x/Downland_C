/* KallistiOS ##version##
*/

// standard headers
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// dc headers
#include <dc/video.h>
#include <dc/maple.h>
#include <dc/sound/sound.h>
#include <dc/sound/sfxmgr.h>
#include <dc/maple/controller.h>

// downland headers
#include "game_types.h"
#include "game.h"
#include "sound.h"
#include "resource_types.h"
#include "../resource_loaders/resource_loader_filesys.h"

GameData gameData;
Resources resources;

sfxhnd_t sounds[SOUND_NUM_SOUNDS];
int playing[SOUND_NUM_SOUNDS];

const char* romFileNames[] = 
{
    "/rd/downland.bin",
    "/rd/downland.rom",
    "/rd/Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc"
};
int romFileNamesCount = sizeof(romFileNames) / sizeof(romFileNames[0]);

uint8_t volume = 128;
#define CENTER 128

void Sound_Play(u8 soundIndex, u8 loop)
{
    int channel = soundIndex * 2;

    if (!loop)
    {
        snd_sfx_play_chn(channel, sounds[soundIndex], volume, CENTER);
        return;
    }

    if (playing[soundIndex] == FALSE)
    {
        sfx_play_data_t data;
        data.chn = channel;
        data.idx = sounds[soundIndex];
        data.vol = volume;
        data.pan = CENTER;
        data.loop = loop;
        data.freq = 0;
        data.loopstart = 0;
        data.loopend = 0;

        snd_sfx_play_ex(&data);

        playing[soundIndex] = TRUE;
    }
}

void Sound_Stop(u8 soundIndex)
{
    int channel = soundIndex * 2;
    playing[soundIndex] = FALSE;

    snd_sfx_stop(channel);
    snd_sfx_stop(channel + 1);
}


void Update_Controls(u8 playerIndex, JoystickState* joystickState)
{
    maple_device_t* cont = maple_enum_type(playerIndex, MAPLE_FUNC_CONTROLLER);

    if (cont == NULL) 
        return;
        
    cont_state_t *state = (cont_state_t *)maple_dev_status(cont);

    if (state == NULL)
        return;

    bool leftDown = (state->buttons & CONT_DPAD_LEFT) || (state->joyx < -64);
    bool rightDown = (state->buttons & CONT_DPAD_RIGHT) || (state->joyx > 64);
    bool upDown = (state->buttons & CONT_DPAD_UP) || (state->joyy < -64);
    bool downDown = (state->buttons & CONT_DPAD_DOWN) || (state->joyy > 64);
    bool jumpDown = state->buttons & CONT_A;
    bool startDown = state->buttons & CONT_START;

    joystickState->leftPressed = !joystickState->leftDown & leftDown;
    joystickState->rightPressed = !joystickState->rightDown & rightDown;
    joystickState->upPressed = !joystickState->upDown & upDown;
    joystickState->downPressed =  !joystickState->downDown & downDown;
    joystickState->jumpPressed =  !joystickState->jumpDown & jumpDown;
    joystickState->startPressed = !joystickState->startDown & startDown;

    joystickState->leftReleased = joystickState->leftDown & !leftDown;
    joystickState->rightReleased = joystickState->rightDown & !rightDown;
    joystickState->upReleased = joystickState->upDown & !upDown;
    joystickState->downReleased =  joystickState->downDown & !downDown;
    joystickState->jumpReleased =  joystickState->jumpDown & !jumpDown;
    joystickState->startReleased = joystickState->startPressed & !startDown;

    joystickState->leftDown = leftDown;
    joystickState->rightDown = rightDown;
    joystickState->upDown = upDown;
    joystickState->downDown = downDown;
    joystickState->jumpDown = jumpDown;
    joystickState->startDown = startDown;
}

void updateFramebufferTexture(const u8* gameFramebuffer, 
                              u16* dcFramebuffer) 
{
    // Convert 1-bit buffer to 16-bit pixels
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) 
    {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) 
        {
            u8 bit = (gameFramebuffer[(x / 8) + (y * FRAMEBUFFER_PITCH)] >> (7 - (x % 8))) & 1;
            dcFramebuffer[(y + 24) * 320 + (x + 32)] = bit ? 0xFFFF : 0x0000; // White or Black
        }
    }
}

// take the framebuffer and apply basic CRT artifacts, updating a
// second framebuffer and a texture for it.
enum CrtColor
{
    CrtColor_Blue,
    CrtColor_Orange
};

void convert1bppImageTo16bppCrtEffectImage(const u8* originalImage,
                                           u16* crtImage,
                                           enum CrtColor crtColor) 
{
    const u8 bytesPerRow = FRAMEBUFFER_WIDTH / 8;

    // Color definitions
    const u16 BLACK  = 0x0000; // 00 black
    const u16 BLUE   = crtColor == CrtColor_Blue ? 0x001F : 0xFC80; // 01 blue
    const u16 ORANGE = crtColor == CrtColor_Blue ? 0xFC80 : 0x001F; // 10 orange
    const u16 WHITE  = 0xFFFF; // 11 white

    for (int y = 0; y < FRAMEBUFFER_HEIGHT; ++y) 
    {
        // every pair of bits generates a color for the two corresponding
        // pixels of the destination texture, so:
        // source bits:        00 01 10 11
        // final pixel colors: black, black, blue, blue, orange, orange, white, white.
        u32 yOffset = (y + 24) * 320;

        for (int x = 0; x < FRAMEBUFFER_WIDTH; x += 2) 
        {
            int byteIndex = (y * bytesPerRow) + (x / 8);
            int bitOffset = 7 - (x % 8);

            // Read two adjacent bits
            uint8_t bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
            uint8_t bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

            // Determine base color
            uint32_t color = BLACK;
            if (bit1 == 0 && bit2 == 1) color = BLUE;
            else if (bit1 == 1 && bit2 == 0) color = ORANGE;
            else if (bit1 == 1 && bit2 == 1) color = WHITE;

            // Apply base color
            crtImage[yOffset + (x + 32)]     = color;
            crtImage[yOffset + (x + 32) + 1] = color;
        }

        // Apply a quick and dirty crt artifact effect
        // pixels whose original bits are adjacent are converted to white
        // source colors: black, black, blue, blue, orange, orange, white, white.
        // source bits:  00 01 10 11
        // seen as:      00 00 01 01 10 10 11 11 // forth and fifth pairs have adjacent bits. 
        //                                          Turn both corresponding pixels to white.
        //                                          Also turn off the other pixel in the pair to
        //                                          black.
        // final final:  black, black, black, white, white, black, white, white
        for (int x = 32; x < (FRAMEBUFFER_WIDTH + 32); x += 2) 
        {
            uint32_t leftPixel = crtImage[yOffset + x];
            uint32_t rightPixel = crtImage[yOffset + x + 1];

            if (rightPixel == BLUE && x < 320 - 2)
            {
                uint32_t pixel3 = crtImage[yOffset + x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    rightPixel = WHITE;
                    leftPixel = BLACK;
                }
            }
            else if (leftPixel == ORANGE && x >= 2)
            {
                uint32_t pixel0 = crtImage[yOffset + x - 1];
                if (pixel0 == BLUE || pixel0 == WHITE)
                {
                    leftPixel = WHITE;
                    rightPixel = BLACK;
                }
            }

            crtImage[yOffset + x] = leftPixel;
            crtImage[yOffset + x + 1] = rightPixel;
        }
    }
}

int main(int argc, char **argv) 
{
    vid_set_mode(DM_320x240, PM_RGB565);
    snd_init();

    bool romFoundAndLoaded = false;
    for (int loop = 0; loop < romFileNamesCount; loop++)
    {
        if (ResourceLoaderFileSys_Init(romFileNames[loop], &resources))
        {
            romFoundAndLoaded = true;
            break;
        }
    }

    assert(romFoundAndLoaded);

    // Load wav files found in romdisk
    sounds[SOUND_JUMP] = snd_sfx_load("/rd/jump_hq.wav");
    sounds[SOUND_LAND] = snd_sfx_load("/rd/land_hq.wav");
    sounds[SOUND_SCREEN_TRANSITION] = snd_sfx_load("/rd/transition_hq.wav");
    sounds[SOUND_SPLAT] = snd_sfx_load("/rd/splat_hq.wav");
    sounds[SOUND_PICKUP] = snd_sfx_load("/rd/pickup_hq.wav");
    sounds[SOUND_RUN] = snd_sfx_load("/rd/run_hq.wav");
    sounds[SOUND_CLIMB_UP] = snd_sfx_load("/rd/climb_up_hq.wav");
    sounds[SOUND_CLIMB_DOWN] = snd_sfx_load("/rd/climb_down_hq.wav");

    for (int loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
        playing[loop] = FALSE;

    for (int loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
        assert(sounds[loop] != SFXHND_INVALID);

    Game_Init(&gameData, &resources);

    u16* crtFramebuffer = malloc(320*240);
    memset(crtFramebuffer, 0, 320*240*2);
    

    while (1) 
    {
        int controllerIndex = 0;
        if (gameData.currentPlayerData != NULL)
        {
            controllerIndex = gameData.currentPlayerData->playerNumber;
        }

        Update_Controls(controllerIndex, &gameData.joystickState);

        if (gameData.joystickState.startPressed)
        {
            gameData.paused = !gameData.paused;
        }

        if (!gameData.paused)
        {
            Game_Update(&gameData, &resources);
        }

        /* Wait for VBlank */
        vid_waitvbl();
        
        //updateFramebufferTexture(gameData.framebuffer, vram_s);

        convert1bppImageTo16bppCrtEffectImage(gameData.framebuffer,
                                              crtFramebuffer,
                                              CrtColor_Blue);

        memcpy(vram_s, crtFramebuffer, 320*240*2);

        vid_flip(-1);
    }

    return 0;
}
