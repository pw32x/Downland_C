#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../game/base_defines.h"
#include "../game/base_types.h"
#include "../game/game.h"
#include "../game/framebuffer_utils.h"
#include "../resources/resources.h"
#include "../misc/screenshot_data.h"
#include "sdl_utils.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

#define SCREEN_SCALE 3
#define SCREEN_WIDTH (FRAMEBUFFER_WIDTH * SCREEN_SCALE)
#define SCREEN_HEIGHT (FRAMEBUFFER_HEIGHT * SCREEN_SCALE)

SDL_Texture* framebufferTexture = NULL;
SDL_Texture* crtFramebufferTexture = NULL;

Resources resources;
GameData gameData;

const char* romFilePath = "downland.bin";

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Downland_C", "1.0", "com.example.Downland_C");

    if (!SDL_Init(SDL_INIT_VIDEO)) 
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Downland_C", 
                                     SCREEN_WIDTH, 
                                     SCREEN_HEIGHT, 
                                     0, 
                                     &window, 
                                     &renderer)) 
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    framebufferTexture = SDL_CreateTexture(renderer, 
                                           SDL_PIXELFORMAT_XRGB8888, 
                                           SDL_TEXTUREACCESS_STREAMING, 
                                           FRAMEBUFFER_WIDTH, 
                                           FRAMEBUFFER_HEIGHT);

    SDL_SetTextureScaleMode(framebufferTexture, SDL_SCALEMODE_NEAREST); // no smoothing

    // Create the texture
    crtFramebufferTexture = SDL_CreateTexture(renderer, 
                                              SDL_PIXELFORMAT_XRGB8888, 
                                              SDL_TEXTUREACCESS_STREAMING, 
                                              FRAMEBUFFER_WIDTH, 
                                              FRAMEBUFFER_HEIGHT);

    SDL_SetTextureScaleMode(crtFramebufferTexture, SDL_SCALEMODE_NEAREST); // no smoothing

    if (!Resources_Init(romFilePath, &resources))
        return SDL_APP_FAILURE;

    Game_Init(&gameData, &resources);

    // setup screenshot
    memcpy(gameData.framebuffer, screenshot_data, FRAMEBUFFER_SIZE);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Game_Update(&gameData);

    // Render to screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Update texture from framebuffer
    SDLUtils_updateFramebufferTexture(gameData.framebuffer, framebufferTexture); 
    SDL_RenderTexture(renderer, framebufferTexture, NULL, NULL);

    // Update texture from crtFramebuffer
    SDLUtils_updateCrtFramebufferAndTexture(gameData.framebuffer,
                                            gameData.crtFramebuffer,
                                            crtFramebufferTexture,
                                            renderer);

    SDL_RenderTexture(renderer, crtFramebufferTexture, NULL, NULL);

    /*
    // write debug text
    SDL_SetRenderScale(renderer, 1.5f, 1.5f);
    SDL_SetRenderDrawColor(renderer, 51, 102, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, 10.0f, 10.0f, "Some debug text");
    SDL_RenderDebugTextFormat(renderer, 10.0f, 20.0f, "This is line number %d", 2);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    */

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */

    SDL_DestroyTexture(framebufferTexture);

    Game_Shutdown(&gameData);
    Resources_Shutdown(&resources);
}