#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "resource_types.h"
#include "sound.h"
}

#include "game_runner.hpp"

using namespace SRL::Types;
using namespace SRL::Input;
using namespace SRL::Math;


extern "C"
{

void* dl_alloc(u32 size)
{
    return (void*)new u8[size];
}

void Sound_Play(u8 soundIndex, u8 loop)
{
	//soundManager.play(soundIndex, loop);
}

void Sound_Stop(u8 soundindex)
{
    //soundManager.stop(soundindex);
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
    bool jumpDown = port->IsHeld(Digital::Button::A) || port->IsHeld(Digital::Button::B) || port->IsHeld(Digital::Button::C);
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
}

Resources* g_resources;
GameRunner* g_gameRunner;

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

    u8* frameBuffer8bpp = new u8[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

    ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(g_gameRunner->m_gameData.cleanBackground,
                                                                     frameBuffer8bpp,
                                                                     FRAMEBUFFER_WIDTH,
                                                                     FRAMEBUFFER_HEIGHT,
                                                                     ImageUtils::ImageConverter::CrtColor::Blue);

    BitmapUtils::InMemoryBitmap* framebufferBitmap = new BitmapUtils::InMemoryBitmap(frameBuffer8bpp, 
                                                                                     FRAMEBUFFER_WIDTH, 
                                                                                     FRAMEBUFFER_HEIGHT, 
                                                                                     PaletteUtils::g_downlandPalette, 
                                                                                     PaletteUtils::g_downlandPaletteColorsCount);

    SRL::Tilemap::Interfaces::Bmp2Tile* tileSet = new SRL::Tilemap::Interfaces::Bmp2Tile(*framebufferBitmap);
    SRL::VDP2::NBG0::LoadTilemap(*tileSet);
    
    delete tileSet;//free work RAM
    delete [] frameBuffer8bpp;
    delete framebufferBitmap;

    /*
    TestTilebin = new SRL::Tilemap::Interfaces::CubeTile("FOG256.BIN");//Load fog tilemap from cd to work RAM
    SRL::VDP2::NBG1::LoadTilemap(*TestTilebin);//Transfer tilemap from work RAM to VDP2 VRAM and register with NBG1
    delete TestTilebin;//free work RAM
   

    //Demonstrate NBG2 loading with Tilemap converted from Bitmap:
    SRL::Bitmap::TGA* logo = new SRL::Bitmap::TGA("LOGO1.TGA");//Load Bitmap image to work RAM
    SRL::Tilemap::Interfaces::Bmp2Tile* TestTilebmp = new SRL::Tilemap::Interfaces::Bmp2Tile(*logo);//convert bitmap to tilemap
    SRL::VDP2::NBG2::LoadTilemap(*TestTilebmp);//Transfer tilemap from work RAM to VDP2 VRAM and register with NBG2
    delete TestTilebmp;//free tilemap from work ram 
    delete logo;//free original bitmap from work ram
    //store XY screen positions of Background scrolls:
    */
    Vector2D Nbg0Position(0.0, 0.0);
    /*
    Vector2D Nbg1Position(0.0, 0.0);
    Vector2D Nbg2Position(-64.0, -16.0);
    */
    SRL::VDP2::NBG0::SetPriority(SRL::VDP2::Priority::Layer2);//set NBG0 priority
    SRL::VDP2::NBG0::ScrollEnable();//enable display of NBG0 

    /*
    SRL::VDP2::NBG1::SetPriority(SRL::VDP2::Priority::Layer6);//set NBG1 priority
    SRL::VDP2::NBG1::SetOpacity(0.5);//set opacity of NBG1
    SRL::VDP2::NBG1::TransparentDisable();//disable fully transparent pixels on Fog(its all half transparent)  
    SRL::VDP2::NBG1::ScrollEnable();//enable display of NBG1 
    
    SRL::VDP2::NBG2::SetPriority(SRL::VDP2::Priority::Layer4);// Set NBG2 priority between NBG0 and NBG1
    SRL::VDP2::NBG2::SetPosition(Nbg2Position);//Set the static screen position for SRL Logo
    SRL::VDP2::NBG2::ScrollEnable();//enable display of NBG2
    */
    //SRL::Debug::Print(1,13,"ColorMode %d", myBitmap.m_bitmapInfo.ColorMode);
    //SRL::Debug::Print(1,14,"Color count %d", myBitmap.m_bitmapInfo.Palette->Count);
    //SRL::Debug::Print(1,22,"sizeof(size_t) %d", sizeof(size_t));

    //SRL::Debug::Print(1,12,"width %d", g_gameRunner->m_dropSprite.m_width);
	//SRL::Debug::Print(1,13,"height %d", g_gameRunner->m_dropSprite.m_height);
	//SRL::Debug::Print(1,14,"num frames %d", g_gameRunner->m_dropSprite.m_numFrames);
    //SRL::Debug::Print(1,15,"frame tex index %d", g_gameRunner->m_dropSprite.m_frameTextureIndexes[0]);
    //SRL::Debug::Print(1,16,"frame tex index %d", g_gameRunner->m_dropSprite.m_frameTextureIndexes[1]);

    //SRL::Debug::Print(1,13,"GameData size %d", sizeof(GameData));
    //SRL::Debug::Print(1,14,"Resources size %d", sizeof(Resources));

    //SRL::Debug::Print(1,17,"player: %d", (u8)resources->roomResources[0].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,18,"player: %d", (u8)resources->roomResources[1].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,19,"player: %d", (u8)resources->roomResources[2].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,20,"player: %d", (u8)resources->roomResources[3].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,21,"player: %d", (u8)resources->roomResources[4].backgroundDrawData.drawCommandCount);

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
        }

        if (!gameData->paused)
        {
            g_gameRunner->update();
        }

        SRL::Core::Synchronize();

        //SRL::Debug::Print(1,17,"x %02d", drop->x << 1);
	    //SRL::Debug::Print(1,18,"y %02d", drop->y >> 8);
        g_gameRunner->draw();

        //move positions of NBG0 and NBG1 scrolls:
        //Nbg0Position += Vector2D(1.0, 1.0);
        //Nbg1Position += Vector2D(-2.0, 1.0);
        //SRL::VDP2::NBG0::SetPosition(Nbg0Position);
        //SRL::VDP2::NBG1::SetPosition(Nbg1Position);
    }
    return 0;
}
