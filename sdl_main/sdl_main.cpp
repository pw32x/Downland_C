#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

extern "C"
{
#include "../game/base_defines.h"
#include "../game/base_types.h"
#include "../game/game.h"
#include "../game/game_types.h"
#include "../game/resource_loader.h"
#include "../game/physics_utils.h"
#include "../game/debug_utils.h"
#include "../game/draw_utils.h"
#include "../game/sound.h"
}

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gamepad.h>

#include "sdl_utils.h"
#include "sdl_sound_manager.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

#define SCREEN_SCALE 3
#define SCREEN_WIDTH (FRAMEBUFFER_WIDTH * SCREEN_SCALE)
#define SCREEN_HEIGHT (FRAMEBUFFER_HEIGHT * SCREEN_SCALE)

SDL_Texture* framebufferTexture = NULL;
SDL_Texture* crtFramebufferTexture = NULL;
SDL_Texture* debugFramebufferTexture = NULL;

BOOL stepFrame = false;

Resources resources;
GameData gameData;

SDLSoundManager soundManager;

// implement the sound function here
void Sound_Play(u8 soundIndex)
{
	soundManager.play(soundIndex);
}


#define TARGET_FPS 60
const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

Uint64 gameStartTime;
Uint64 timeFrequency;
Uint64 frameCount = 0;
float fps;

const char* romFilePath = "downland.bin";

SDL_Gamepad* gamePad = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Downland_C", "1.0", "com.example.Downland_C");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) 
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

    // Create the debug texture
    debugFramebufferTexture = SDL_CreateTexture(renderer, 
                                                SDL_PIXELFORMAT_ARGB8888, 
                                                SDL_TEXTUREACCESS_STREAMING, 
                                                FRAMEBUFFER_WIDTH, 
                                                FRAMEBUFFER_HEIGHT);

    SDL_SetTextureScaleMode(debugFramebufferTexture, SDL_SCALEMODE_NEAREST); // no smoothing

    if (!ResourceLoader_Init(romFilePath, &resources))
        return SDL_APP_FAILURE;

    soundManager.init();

    // load these in the same order as the IDs in game\sound.h
    soundManager.loadSound("jump.wav");
    soundManager.loadSound("land.wav");
    soundManager.loadSound("pickup.wav");
    soundManager.loadSound("transition.wav");
    soundManager.loadSound("splat.wav");

    Game_Init(&gameData, &resources);

    gameStartTime = SDL_GetPerformanceCounter();
    timeFrequency = SDL_GetPerformanceFrequency();

    srand((unsigned int)gameStartTime);

    if (SDL_HasGamepad())
    {
        int joystickCount;
        SDL_JoystickID* joystickIds = SDL_GetGamepads(&joystickCount);

	    if (joystickIds && joystickCount > 0)
        {
            gamePad = SDL_OpenGamepad(joystickIds[0]);
        }
    }

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
            gameData.paused = !gameData.paused;

            if (gameData.paused)
                soundManager.pause();
            else
                soundManager.resume();
        }

#ifdef DEV_MODE
        else if (event->key.key == SDLK_SPACE && gameData.currentPlayerData->currentRoom->roomNumber != TITLESCREEN_ROOM_INDEX)
        {
            stepFrame = TRUE;
            gameData.paused = FALSE;
            soundManager.resume();
        }

        if (event->key.key == SDLK_GRAVE)
        {
            Game_Init(&gameData, &resources);
            Game_TransitionToRoom(&gameData, TITLESCREEN_ROOM_INDEX, &resources);
        }
        else if (event->key.key >= SDLK_1 && event->key.key <= SDLK_9)
        {
            Game_Init(&gameData, &resources);
            u8 roomNumber = event->key.key - SDLK_1;
            gameData.currentPlayerData->lastDoor = &resources.roomResources[roomNumber].doorInfoData.doorInfos[0];
            Game_TransitionToRoom(&gameData, roomNumber, &resources);
        }
        else if (event->key.key == SDLK_0)
        {
            Game_Init(&gameData, &resources);
            gameData.currentPlayerData->lastDoor = &resources.roomResources[9].doorInfoData.doorInfos[0];
            Game_TransitionToRoom(&gameData, 9, &resources);
        }
#endif
    }

    return SDL_APP_CONTINUE;
}

void Update_Controls(JoystickState* joystickState)
{
    const bool* currentKeyStates = SDL_GetKeyboardState(NULL);

    bool leftDown = currentKeyStates[SDL_SCANCODE_LEFT];
    bool rightDown = currentKeyStates[SDL_SCANCODE_RIGHT];
    bool upDown = currentKeyStates[SDL_SCANCODE_UP];
    bool downDown = currentKeyStates[SDL_SCANCODE_DOWN];
    bool jumpDown = currentKeyStates[SDL_SCANCODE_LCTRL] || currentKeyStates[SDL_SCANCODE_Z] || currentKeyStates[SDL_SCANCODE_LSHIFT];
    bool startDown = FALSE;

    if (gamePad != NULL)
    {
        leftDown |= SDL_GetGamepadButton(gamePad, SDL_GAMEPAD_BUTTON_DPAD_LEFT) || 
                    SDL_GetGamepadAxis(gamePad, SDL_GAMEPAD_AXIS_LEFTX) < -10000;

        rightDown |= SDL_GetGamepadButton(gamePad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT) || 
                     SDL_GetGamepadAxis(gamePad, SDL_GAMEPAD_AXIS_LEFTX) > 10000;

        upDown |= SDL_GetGamepadButton(gamePad, SDL_GAMEPAD_BUTTON_DPAD_UP) ||
                  SDL_GetGamepadAxis(gamePad, SDL_GAMEPAD_AXIS_LEFTY) < -10000;

        downDown |= SDL_GetGamepadButton(gamePad, SDL_GAMEPAD_BUTTON_DPAD_DOWN) ||
                  SDL_GetGamepadAxis(gamePad, SDL_GAMEPAD_AXIS_LEFTY) > 10000;

        jumpDown |= SDL_GetGamepadButton(gamePad, SDL_GAMEPAD_BUTTON_SOUTH);
        startDown |= SDL_GetGamepadButton(gamePad, SDL_GAMEPAD_BUTTON_START);
    }

    joystickState->leftPressed = !joystickState->leftDown & leftDown;
    joystickState->rightPressed = !joystickState->rightDown & rightDown;
    joystickState->upPressed = !joystickState->upDown & upDown;
    joystickState->downPressed =  !joystickState->downDown & downDown;
    joystickState->jumpPressed =  !joystickState->jumpDown & jumpDown;
    joystickState->startDownPressed = !joystickState->startDown & startDown;

    joystickState->leftReleased = joystickState->leftDown & !leftDown;
    joystickState->rightReleased = joystickState->rightDown & !rightDown;
    joystickState->upReleased = joystickState->upDown & !upDown;
    joystickState->downReleased =  joystickState->downDown & !downDown;
    joystickState->jumpReleased =  joystickState->jumpDown & !jumpDown;
    joystickState->startDownReleased = joystickState->startDownPressed & !startDown;

    joystickState->leftDown = leftDown;
    joystickState->rightDown = rightDown;
    joystickState->upDown = upDown;
    joystickState->downDown = downDown;
    joystickState->jumpDown = jumpDown;
    joystickState->startDown = startDown;

#ifdef DEV_MODE
    bool debugStateDown = currentKeyStates[SDL_SCANCODE_TAB];

    if (gamePad != NULL)
    {
        debugStateDown |= SDL_GetGamepadButton(gamePad, SDL_GAMEPAD_BUTTON_EAST);
    }

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
}

// Function to convert an 8-bit value to a binary string
void uint8_to_binary_str(uint8_t value, char *str) {
    for (int i = 7; i >= 0; i--) {
        str[7 - i] = (value & (1 << i)) ? '1' : '0';
    }
    str[8] = '\0'; // Null-terminate the string
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Uint64 frameStartTime = SDL_GetPerformanceCounter();

    Update_Controls(&gameData.joystickState);

    if (gameData.joystickState.startDownPressed)
    {
        gameData.paused = !gameData.paused;

        if (gameData.paused)
            soundManager.pause();
        else
            soundManager.resume();
    }

    if (!gameData.paused)
    {
        memset(debugFramebuffer, 0, sizeof(debugFramebuffer));
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


    SDLUtils_updateDebugFramebufferTexture(debugFramebuffer, 
                                           debugFramebufferTexture);

    SDL_RenderTexture(renderer, debugFramebufferTexture, NULL, NULL);

    frameCount++;
	Uint64 currentTime = SDL_GetPerformanceCounter();
    float elapsedSeconds = (float)(currentTime - gameStartTime) / timeFrequency;

    if (elapsedSeconds >= 1.0f) // Every second
    {  
        fps = frameCount / elapsedSeconds;
        frameCount = 0;
        gameStartTime = currentTime;
    }
     
#ifdef DEV_MODE
    // write debug text
    SDL_SetRenderScale(renderer, 1.5f, 1.5f);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    // SDL_RenderDebugText(renderer, 10.0f, 10.0f, "Some debug text");
    /*
    SDL_RenderDebugTextFormat(renderer, 10.0f, 20.0f, "Fps: %f", fps);

    SDL_RenderDebugTextFormat(renderer, 10.0f, 30.0f, "left: %d", gameData.joystickState.leftDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 40.0f, "right: %d", gameData.joystickState.rightDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 50.0f, "up: %d", gameData.joystickState.upDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 60.0f, "down: %d", gameData.joystickState.downDown);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 70.0f, "jump: %d", gameData.joystickState.jumpDown);

    */

    //SDL_RenderDebugTextFormat(renderer, 10.0f, 20.0f, "x: %x", gameData.playerData.x);
    //SDL_RenderDebugTextFormat(renderer, 10.0f, 30.0f, "y: %x", gameData.playerData.y);

    //SDL_RenderDebugTextFormat(renderer, 10.0f, 20.0f, "regen time: %x", gameData.playerData.regenerationCounter);
    //SDL_RenderDebugTextFormat(renderer, 10.0f, 30.0f, "cantmove time: %x", gameData.playerData.cantMoveCounter);

    
    /*
    SDL_RenderDebugTextFormat(renderer, 10.0f, 50.0f, "fallcounter: %x", gameData.ballData.fallStateCounter);
    */

    char binary_str[9];
    uint8_to_binary_str(leftPixelData, binary_str);

    char binary_str2[9];
    uint8_to_binary_str(rightPixelData, binary_str2);

    //SDL_RenderDebugTextFormat(renderer, 10.0f, 20.0f, "%s%s", binary_str, binary_str2);

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

    if (stepFrame)
    {
        gameData.paused = TRUE;
        stepFrame = FALSE;
        soundManager.pause();
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */

    if (gamePad != NULL) 
        SDL_CloseGamepad(gamePad);

    SDL_DestroyTexture(framebufferTexture);

    Game_Shutdown(&gameData);
    soundManager.shutdown();
    ResourceLoader_Shutdown(&resources);
}