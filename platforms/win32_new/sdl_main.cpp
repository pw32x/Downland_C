// This implements basic SDL support

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

extern "C"
{
#include "../../game/base_defines.h"
#include "../../game/base_types.h"
#include "../../game/game.h"
#include "../../game/game_types.h"
#include "../../resource_loaders/resource_loader_filesys.h"
#include "../../game/physics_utils.h"
#include "../../game/debug_utils.h"
#include "../../game/draw_utils.h"
#include "../../game/dl_sound.h"
#include "../../game/alloc.h"
#include "../../game/dl_rand.h"
}

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gamepad.h>

#include "video_filters\sdl_video_filter_utils.h"
#include "video_filters\sdl_video_filter_raw.h"
#include "video_filters\sdl_video_filter_basic_crt_artifacts_blue.h"
#include "video_filters\sdl_video_filter_basic_crt_artifacts_orange.h"
#include "video_filters\sdl_video_filter_new_renderer.h"

#include "..\..\sdl_sound\sdl_sound_manager.h"

#include <vector>

const char* appName = "Downland_C";
const char* windowTitle = "Downland_C - https://github.com/pw32x/Downland_C";
const char* appIdentifier = "com.example.Downland_C";

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

#define SCREEN_SCALE 3
#define SCREEN_WIDTH (FRAMEBUFFER_WIDTH * SCREEN_SCALE)
#define SCREEN_HEIGHT (FRAMEBUFFER_HEIGHT * SCREEN_SCALE)

int screenWidth = SCREEN_WIDTH;
int screenHeight = SCREEN_HEIGHT;

#ifdef DEV_MODE
SDL_Texture* debugFramebufferTexture = NULL;
#endif

BOOL stepFrame = false;

Resources resources;
GameData gameData;

int currentVideoFilterIndex = -1;
std::vector<std::unique_ptr<SDLVideoFilterBase>> videoFilters;

SDLSoundManager soundManager;

bool isFullscreen = false;
SDL_FRect destRect;

// implement the sound function here
void Sound_Play(u8 soundIndex, u8 loop)
{
	soundManager.play(soundIndex, loop);
}

void Sound_Stop(u8 soundindex)
{
    soundManager.stop(soundindex);
}

Uint64 gameStartTime;
Uint64 timeFrequency;
Uint64 frameCount = 0;
float fps;

const char* romFileNames[] = 
{
    "downland.bin",
    "downland.rom",
    "Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc"
};

int romFileNamesCount = sizeof(romFileNames) / sizeof(romFileNames[0]);

SDL_Gamepad* gamePad = NULL;

void selectFilter(int videoFilterIndex)
{
    if (currentVideoFilterIndex != -1)
    {
        videoFilters[currentVideoFilterIndex]->shutdown();
    }

    currentVideoFilterIndex = videoFilterIndex;
}

void updateDestRect()
{
    // Get the current rendering dimentions and recompute
    // the destination rect for the rendering of the game to
    // maintain the correct aspect ratio.
    SDL_DisplayID displayId = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* displayMode = SDL_GetDesktopDisplayMode(displayId);

    if (isFullscreen)
    {
        screenWidth = displayMode->w;
        screenHeight = displayMode->h;
    }
    else
    {
        SDL_GetWindowSize(window, &screenWidth, &screenHeight);
    }
    
    SDLUtils_computeDestinationRect(screenWidth, 
                                    screenHeight,
                                    FRAMEBUFFER_WIDTH,
                                    FRAMEBUFFER_HEIGHT,
                                    &destRect);
}

void setFullscreen(bool fullscreen)
{
    SDL_SetWindowFullscreen(window, fullscreen);
    isFullscreen = fullscreen;

    updateDestRect();
}

void gameRoomChanged(const GameData* gameData, u8 roomNumber, s8 transitionType)
{
    auto& currentVideoFilter = videoFilters[currentVideoFilterIndex];
    currentVideoFilter->roomChanged(gameData, roomNumber, transitionType);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // Init the app and main window
    // 

    SDL_SetAppMetadata(appName, "1.0", appIdentifier);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) 
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(windowTitle, 
                                     screenWidth, 
                                     screenHeight, 
                                     0 , 
                                     &window, 
                                     &renderer)) 
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetWindowResizable(window, true);

#ifdef DEV_MODE
    // Create the debug texture
    debugFramebufferTexture = SDL_CreateTexture(renderer, 
                                                SDL_PIXELFORMAT_ARGB8888, 
                                                SDL_TEXTUREACCESS_STREAMING, 
                                                FRAMEBUFFER_WIDTH, 
                                                FRAMEBUFFER_HEIGHT);

    SDL_SetTextureScaleMode(debugFramebufferTexture, SDL_SCALEMODE_NEAREST); // no smoothing
#endif

    // Load game resources from the rom
    // 

    bool romFoundAndLoaded = false;
    for (int loop = 0; loop < romFileNamesCount; loop++)
    {
        if (ResourceLoaderFileSys_Init(romFileNames[loop], &resources))
        {
            romFoundAndLoaded = true;
            break;
        }
    }

    if (!romFoundAndLoaded)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
                                 appName, 
                                 "Downland V1.1 rom file not found. \n "\
                                 "'downland.rom',\n 'downland.bin', or \n"\
                                 "'Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc'\n "\
                                 "not found in the same folder as the Downland_C.exe", 
                                 window);
        return SDL_APP_FAILURE;
    }

    // Init sound
    // 

    soundManager.init();

    // load these in the same order as the IDs in game\sound.h
    soundManager.loadSound("jump.wav");
    soundManager.loadSound("land.wav");
    soundManager.loadSound("transition.wav");
    soundManager.loadSound("splat.wav");
    soundManager.loadSound("pickup.wav");
    soundManager.loadSound("run.wav");
    soundManager.loadSound("climb_up.wav");
    soundManager.loadSound("climb_down.wav");

    // Init Game

    Game_Init(&gameData, &resources);

    // Init timers and random numbers
    // 

    gameStartTime = SDL_GetPerformanceCounter();
    timeFrequency = SDL_GetPerformanceFrequency();

    dl_srand((unsigned int)gameStartTime);

    // Init joystick
    // 

    if (SDL_HasGamepad())
    {
        int joystickCount;
        SDL_JoystickID* joystickIds = SDL_GetGamepads(&joystickCount);

	    if (joystickIds && joystickCount > 0)
        {
            gamePad = SDL_OpenGamepad(joystickIds[0]);
        }
    }

    // Init video filters
    // 
    videoFilters.emplace_back(std::make_unique<SDLVideoFilterRaw>(renderer, &resources));
    videoFilters.emplace_back(std::make_unique<SDLVideoFilterBasicCrtArtifactsBlue>(renderer, &resources));
    videoFilters.emplace_back(std::make_unique<SDLVideoFilterBasicCrtArtifactsOrange>(renderer, &resources));
    videoFilters.emplace_back(std::make_unique<SDLVideoFilterNewRenderer>(renderer, &resources));

    selectFilter((int)videoFilters.size() - 1);

    Game_ChangedRoomCallback = gameRoomChanged;

    setFullscreen(false);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    const bool* currentKeyStates = SDL_GetKeyboardState(NULL);

    if (event->type == SDL_EVENT_QUIT) 
    {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_WINDOW_RESIZED)
    {
        updateDestRect();
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        if (event->key.key == SDLK_ESCAPE)
        {
            gameData.paused = !gameData.paused;

            if (gameData.paused)
                soundManager.pauseAll();
            else
                soundManager.resumeAll();
        }
        else if (event->key.key == SDLK_F1)
        {
            int nextVideoFilterIndex = currentVideoFilterIndex + 1;

            if (nextVideoFilterIndex >= (int)videoFilters.size())
                nextVideoFilterIndex = 0;

            selectFilter(nextVideoFilterIndex);
        }
        else if (event->key.key == SDLK_RETURN && currentKeyStates[SDL_SCANCODE_LALT])
        {
            setFullscreen(!isFullscreen);
        }

#ifdef DEV_MODE
        else if (event->key.key == SDLK_SPACE && gameData.currentPlayerData->currentRoom->roomNumber != TITLESCREEN_ROOM_INDEX)
        {
            stepFrame = TRUE;
            gameData.paused = FALSE;
            soundManager.resumeAll();
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

            if (gameData.currentPlayerData == NULL)
            {
                Game_InitPlayers(&gameData, &resources);
            }

            gameData.currentPlayerData->lastDoor = &resources.roomResources[roomNumber].doorInfoData.doorInfos[0];

            Game_TransitionToRoom(&gameData, roomNumber, &resources);
        }
        else if (event->key.key == SDLK_0)
        {
            Game_Init(&gameData, &resources);

            if (gameData.currentPlayerData == NULL)
            {
                Game_InitPlayers(&gameData, &resources);
            }

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

    // Update the controls even if paused, to 
    // handle the start (pause/unpause) button.

    Update_Controls(&gameData.joystickState);

    if (gameData.joystickState.startPressed)
    {
        gameData.paused = !gameData.paused;

        if (gameData.paused)
            soundManager.pauseAll();
        else
            soundManager.resumeAll();
    }

    // Process the game 
    // 

    if (!gameData.paused)
    {
#ifdef DEV_MODE
        memset(debugFramebuffer, 0, sizeof(debugFramebuffer));
#endif
        Game_Update(&gameData, &resources);
    }

    // Render to screen
    // 

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    auto& currentVideoFilter = videoFilters[currentVideoFilterIndex];

    currentVideoFilter->update(&gameData);
    SDL_RenderTexture(renderer, currentVideoFilter->getOutputTexture(), NULL, &destRect);

#ifdef DEV_MODE
    SDLUtils_updateDebugFramebufferTexture(debugFramebuffer, 
                                           debugFramebufferTexture);

    SDL_RenderTexture(renderer, debugFramebufferTexture, NULL, NULL);
#endif

    // Compute elapsed time for the frame
    // 

    frameCount++;
	Uint64 currentTime = SDL_GetPerformanceCounter();
    float elapsedSeconds = (float)(currentTime - gameStartTime) / timeFrequency;

    if (elapsedSeconds >= 1.0f) // Every second
    {  
        fps = frameCount / elapsedSeconds;
        frameCount = 0;
        gameStartTime = currentTime;
    }
     
#ifdef SHOW_DEBUG_TEXT
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

    /*
    if (gameData.currentPlayerData != NULL)
    {
        SDL_RenderDebugTextFormat(renderer, 20.0f, 20.0f, "x:    %x", gameData.currentPlayerData->x);
        SDL_RenderDebugTextFormat(renderer, 20.0f, 30.0f, "y:    %x", gameData.currentPlayerData->y);
        SDL_RenderDebugTextFormat(renderer, 20.0f, 40.0f, "air:  %x", gameData.currentPlayerData->jumpAirCounter);
        SDL_RenderDebugTextFormat(renderer, 20.0f, 50.0f, "spdx: %x", gameData.currentPlayerData->speedx);
        SDL_RenderDebugTextFormat(renderer, 20.0f, 60.0f, "spdy: %x", gameData.currentPlayerData->speedy);

        SDL_RenderDebugTextFormat(renderer, 20.0f, 80.0f, "air momentum: %x", gameData.currentPlayerData->preserveAirMomentum);
    }
    */
    

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


    double targetFrameTime = 1.0 / TARGET_FPS; 
    double remainingTime = targetFrameTime - elapsedSeconds;
        
    if (remainingTime > 0) 
    {
        SDL_Delay((Uint32)(remainingTime * 1000)); // Convert to milliseconds
    }

    // If we're stepping, then automatically pause
    //

    if (stepFrame)
    {
        gameData.paused = TRUE;
        stepFrame = FALSE;
        soundManager.pauseAll();
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */

    if (gamePad != NULL) 
        SDL_CloseGamepad(gamePad);

    Game_Shutdown(&gameData);
    soundManager.shutdown();
    ResourceLoaderFileSys_Shutdown(&resources);
}