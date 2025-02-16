#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../game/base_defines.h"
#include "../game/base_types.h"
#include "../game/game.h"
#include "../game/resource_loader.h"
#include "sdl_utils.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

#define SCREEN_SCALE 3
#define SCREEN_WIDTH (FRAMEBUFFER_WIDTH * SCREEN_SCALE)
#define SCREEN_HEIGHT (FRAMEBUFFER_HEIGHT * SCREEN_SCALE)

SDL_Texture* framebufferTexture = NULL;
SDL_Texture* crtFramebufferTexture = NULL;

BOOL paused = false;

Resources resources;
GameData gameData;

#define TARGET_FPS 60
const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

Uint64 gameStartTime;
Uint64 timeFrequency;
Uint64 frameCount = 0;
float fps;

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
                                     0 , 
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

    if (!ResourceLoader_Init(romFilePath, &resources))
        return SDL_APP_FAILURE;

    Game_Init(&gameData, &resources);

    gameStartTime = SDL_GetPerformanceCounter();
    timeFrequency = SDL_GetPerformanceFrequency();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) 
    {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        if (event->key.key == SDLK_ESCAPE)
        {
            paused = !paused;
        }

        if (event->key.key == SDLK_TAB || 
            event->key.key == SDLK_GRAVE)
        {
            Game_TransitionToRoom(&gameData, TITLESCREEN_ROOM_INDEX, &resources);
        }
        else if (event->key.key >= SDLK_1 && event->key.key <= SDLK_9)
        {
            u8 roomNumber = event->key.key - SDLK_1;
            Game_TransitionToRoom(&gameData, roomNumber, &resources);
        }
        else if (event->key.key == SDLK_0)
        {
            Game_TransitionToRoom(&gameData, 9, &resources);
        }
    }

    return SDL_APP_CONTINUE;
}

void Update_Controls(JoystickState* joystickState)
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    u8 leftDown = currentKeyStates[SDL_SCANCODE_LEFT];
    u8 rightDown = currentKeyStates[SDL_SCANCODE_RIGHT];
    u8 upDown = currentKeyStates[SDL_SCANCODE_UP];
    u8 downDown = currentKeyStates[SDL_SCANCODE_DOWN];
    u8 jumpDown = currentKeyStates[SDL_SCANCODE_SPACE];

    joystickState->leftPressed = !joystickState->leftDown & leftDown;
    joystickState->rightPressed = !joystickState->rightDown & rightDown;
    joystickState->upPressed = !joystickState->upDown & upDown;
    joystickState->downPressed =  !joystickState->downDown & downDown;
    joystickState->jumpPressed =  !joystickState->jumpDown & jumpDown;

    joystickState->leftReleased = joystickState->leftDown & !leftDown;
    joystickState->rightReleased = joystickState->rightDown & !rightDown;
    joystickState->upReleased = joystickState->upDown & !upDown;
    joystickState->downReleased =  joystickState->downDown & !downDown;
    joystickState->jumpReleased =  joystickState->jumpDown & !jumpDown;

    joystickState->leftDown = leftDown;
    joystickState->rightDown = rightDown;
    joystickState->upDown = upDown;
    joystickState->downDown = downDown;
    joystickState->jumpDown = jumpDown;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Uint64 frameStartTime = SDL_GetPerformanceCounter();

    if (!paused)
    {
        Update_Controls(&gameData.joystickState);
        Game_Update(&gameData, &resources);
    }

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

    frameCount++;
	Uint64 currentTime = SDL_GetPerformanceCounter();
    float elapsedSeconds = (float)(currentTime - gameStartTime) / timeFrequency;

    if (elapsedSeconds >= 1.0f) // Every second
    {  
        fps = frameCount / elapsedSeconds;
        frameCount = 0;
        gameStartTime = currentTime;
    }
     
#if 0
    // write debug text
    SDL_SetRenderScale(renderer, 1.5f, 1.5f);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    // SDL_RenderDebugText(renderer, 10.0f, 10.0f, "Some debug text");
    SDL_RenderDebugTextFormat(renderer, 10.0f, 20.0f, "Fps: %f", fps);

    SDL_RenderDebugTextFormat(renderer, 10.0f, 30.0f, "left: %d", gameData.joystickState.leftDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 40.0f, "right: %d", gameData.joystickState.rightDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 50.0f, "up: %d", gameData.joystickState.upDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 60.0f, "down: %d", gameData.joystickState.downDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 70.0f, "jump: %d", gameData.joystickState.jumpDown);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
#endif

    SDL_RenderPresent(renderer);

    // compute the frame's time and wait for the target frame time to pass
    elapsedSeconds = (float)(SDL_GetPerformanceCounter() - frameStartTime) / timeFrequency;
    double remainingTime = TARGET_FRAME_TIME - elapsedSeconds;
        
    if (remainingTime > 0) 
    {
        SDL_Delay((Uint32)(remainingTime * 1000)); // Convert to milliseconds
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */

    SDL_DestroyTexture(framebufferTexture);

    Game_Shutdown(&gameData);
    ResourceLoader_Shutdown(&resources);
}