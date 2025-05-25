/* KallistiOS ##version##

   screenshot.c
   Copyright (C) 2024 Andy Barajas

*/

/* 
   This program demonstrates how to use the vid_screen_shot() function
   to capture and save a screenshot in the PPM format to your computer
   using the DC Tool. This tool requires the '-c "."' command-line argument
   to operate correctly.

   The program cycles through a color gradient background and allows user
   interaction to capture screenshots or exit the program.

   Usage:
   Ensure the '/pc/' directory path is correctly specified in the vid_screen_shot()
   function call so that the screenshot.ppm file is saved in the appropriate
   directory on your computer.
*/

#include <stdio.h>

#include <dc/video.h>
#include <dc/fmath.h>
#include <dc/maple.h>
#include <dc/biosfont.h>
#include <dc/maple/controller.h>

#include <assert.h>

#include <kos/fs.h>

#include <kos/thread.h>

#include "game_types.h"
#include "game.h"
#include "resource_types.h"
#include "../resource_loaders/resource_loader_filesys.h"

GameData gameData;
Resources resources;

const char* romFileNames[] = 
{
    "/rd/downland.bin",
    "/rd/downland.rom",
    "/rd/Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc"
};
int romFileNamesCount = sizeof(romFileNames) / sizeof(romFileNames[0]);

void Sound_Play(u8 soundIndex, u8 loop)
{
}

void Sound_Stop(u8 soundindex)
{
}

int main(int argc, char **argv) 
{
    uint8_t r, g, b;
    uint32_t t = 0;
    //char filename[256];

    /* Adjust frequency for faster or slower transitions */
    float frequency = 0.01; 
    
    //maple_device_t *cont;
    //cont_state_t *state;

    /* Set the video mode */
    vid_set_mode(DM_320x240, PM_RGB565);

/*
    file_t fd = 0;
    fd = fs_open("/rd/Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc", O_RDONLY);
    assert(fd);
*/

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

    Game_Init(&gameData, &resources);

    while (1) 
    {
        /*
        if ((cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER)) != NULL) 
        {
            state = (cont_state_t *)maple_dev_status(cont);

            if(state == NULL)
                break;

            if(state->buttons & CONT_START)
                break;

            if(state->buttons & CONT_A) {
                sprintf(filename, "/pc/screenshot%03d.ppm", counter);
                vid_screen_shot(filename);
                counter = (counter + 1) % 1000;
            }
        }
        */

        Game_Update(&gameData, &resources);

        /* Wait for VBlank */
        vid_waitvbl();
        
        /* Calculate next background color */
        r = (uint8_t)((fsin(frequency * t + 0) * 127.5) + 127.5);
        g = (uint8_t)((fsin(frequency * t + 2 * F_PI / 3) * 127.5) + 127.5);
        b = (uint8_t)((fsin(frequency * t + 4 * F_PI / 3) * 127.5) + 127.5);

        /* Increment t to change color in the next cycle */
        t = (t + 1) % INT32_MAX;

        /* Draw Background */
        vid_clear(r, g, b);


        vid_flip(-1);
    }

    return 0;
}
