#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "resource_types.h"
#include "dl_sound.h"
#include "game.h"
}

#include "game_runner.hpp"
#include "sound_manager.hpp"

using namespace SRL::Types;
using namespace SRL::Input;
using namespace SRL::Math;

#define ENABLE_SOUND

Resources* g_resources;
GameRunner* g_gameRunner;

#ifdef ENABLE_SOUND 
SoundManager* g_soundManager;
#endif

void roomTransitionDone(const GameData* gameData, u8 roomNumber, s8 transitionType)
{
    g_gameRunner->roomTransitionDone(gameData, roomNumber, transitionType);
}

extern "C"
{

void* dl_alloc(u32 size)
{
    return (void*)new u8[size];
}

void Sound_Play(u8 soundIndex, u8 loop)
{
#ifdef ENABLE_SOUND 
    g_soundManager->Play(soundIndex, loop);
#endif
}

void Sound_Stop(u8 soundIndex)
{
#ifdef ENABLE_SOUND 
    g_soundManager->Stop(soundIndex);
#endif
}
}


void Update_Controls(int controllerIndex, 
                     Digital* joystickPorts[2],
                     JoystickState* joystickState)
{
    Digital* port = joystickPorts[controllerIndex];

    if (!port->IsConnected())
        return;

    // Check D-Pad
    bool leftDown = port->IsHeld(Digital::Button::Left);
    bool rightDown = port->IsHeld(Digital::Button::Right);
    bool upDown = port->IsHeld(Digital::Button::Up);
    bool downDown = port->IsHeld(Digital::Button::Down);
    bool jumpDown = port->IsHeld(Digital::Button::A);
    bool startDown = port->IsHeld(Digital::Button::START);

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
    bool debugStateDown = port->IsHeld(Digital::Button::B);

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
}



int main()
{
    SRL::Core::Initialize(HighColor(20,10,50));
    Digital port0(0); // Initialize gamepad on port 0
    Digital port1(1); // Initialize gamepad on port 1
    Digital* joystickPorts[2] = { &port0, &port1 }; // Initialize gamepad on port 0

    g_resources = (Resources*)dl_alloc(sizeof(Resources));

    int result = RESULT_UNKNOWNFAILURE;

    for (int loop = 0; loop < romFileNamesCount; loop++)
    {
        result = DownlandResourceLoader::LoadResources(romFileNames[loop], g_resources);

        if (result == RESULT_OK)
        {
            break;
        }
     }

    g_gameRunner = new GameRunner(g_resources);

#ifdef ENABLE_SOUND 
    g_soundManager = new SoundManager();
#endif

    SRL::VDP2::NBG0::SetPriority(SRL::VDP2::Priority::Layer0);//set NBG0 priority
    SRL::VDP2::NBG0::ScrollEnable();//enable display of NBG0 

    Game_TransitionDone = roomTransitionDone;

    //Vector2D scrollOffset = Vector2D(-SCREEN_OFFSET_X, -SCREEN_OFFSET_Y);
    //SRL::VDP2::NBG0::SetPosition(scrollOffset);

    while(1)
    {
        int controllerIndex = 0;

        GameData* gameData = &g_gameRunner->m_gameData;

        if (gameData->currentPlayerData != NULL)
        {
            controllerIndex = gameData->currentPlayerData->playerNumber;
        }

        Update_Controls(controllerIndex, 
                        joystickPorts,
                        &gameData->joystickState);

        if (gameData->joystickState.startPressed)
        {
            gameData->paused = !gameData->paused;

#ifdef ENABLE_SOUND 
            if (gameData->paused)
                g_soundManager->StopAll();
#endif
        }

        if (!gameData->paused)
        {
            g_gameRunner->update();
        }

#ifdef ENABLE_SOUND 
        g_soundManager->Update();
#endif

        SRL::Core::Synchronize();

        g_gameRunner->draw();
    }
    return 0;
}
