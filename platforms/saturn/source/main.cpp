#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "game_types.h"
#include "game.h"
#include "sound.h"
#include "resource_types.h"
#include "sound.h"
}

#include "downland_resource_loader_saturn.hpp"

GameData* gameData = NULL;
Resources* resources = NULL;

const char* romFileNames[] = 
{
    "DOWNLAND.BIN",
    "DOWNLAND.ROM",
};
int romFileNamesCount = sizeof(romFileNames) / sizeof(romFileNames[0]);

extern uint16_t VDP2_CYCA0L;
extern uint16_t VDP2_CYCA0U;
extern uint16_t VDP2_CYCA1L;
extern uint16_t VDP2_CYCA1U;
extern uint16_t VDP2_CYCB0L;
extern uint16_t VDP2_CYCB0U;
extern uint16_t VDP2_CYCB1L;
extern uint16_t VDP2_CYCB1U;

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


int main()
{
    SRL::Core::Initialize(HighColor(20,10,50));
    Digital port0(0); // Initialize gamepad on port 0
  
    gameData = (GameData*)dl_alloc(sizeof(GameData));
    resources = (Resources*)dl_alloc(sizeof(Resources));

    int result = RESULT_UNKNOWNFAILURE;

    for (int loop = 0; loop < romFileNamesCount; loop++)
    {
        result = DownlandResourceLoader::LoadResources(romFileNames[loop], resources);

        if (result == RESULT_OK)
        {
            break;
        }
     }

    Game_Init(gameData, resources);

    SRL::Tilemap::Interfaces::CubeTile* TestTilebin = new SRL::Tilemap::Interfaces::CubeTile("SPACE.BIN");//Load tilemap from cd to work RAM
    SRL::VDP2::NBG0::LoadTilemap(*TestTilebin);//Transfer tilemap from work RAM to VDP2 VRAM and register with NBG0
    delete TestTilebin;//free work RAM

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
    Vector2D Nbg0Position(0.0, 0.0);
    Vector2D Nbg1Position(0.0, 0.0);
    Vector2D Nbg2Position(-64.0, -16.0);

    SRL::VDP2::NBG0::SetPriority(SRL::VDP2::Priority::Layer2);//set NBG0 priority
    SRL::VDP2::NBG0::ScrollEnable();//enable display of NBG0 

    SRL::VDP2::NBG1::SetPriority(SRL::VDP2::Priority::Layer6);//set NBG1 priority
    SRL::VDP2::NBG1::SetOpacity(0.5);//set opacity of NBG1
    SRL::VDP2::NBG1::TransparentDisable();//disable fully transparent pixels on Fog(its all half transparent)  
    SRL::VDP2::NBG1::ScrollEnable();//enable display of NBG1 
    
    SRL::VDP2::NBG2::SetPriority(SRL::VDP2::Priority::Layer4);// Set NBG2 priority between NBG0 and NBG1
    SRL::VDP2::NBG2::SetPosition(Nbg2Position);//Set the static screen position for SRL Logo
    SRL::VDP2::NBG2::ScrollEnable();//enable display of NBG2

    if (result == RESULT_OK)
    {
        SRL::Debug::Print(1,10,"Loading rom all good");
    }
    else
    {
        SRL::Debug::Print(1,10,"Loading failed %d", result);
    }

    //SRL::Debug::Print(1,13,"GameData size %d", sizeof(GameData));
    //SRL::Debug::Print(1,14,"Resources size %d", sizeof(Resources));

    SRL::Debug::Print(1,17,"player: %d", (u8)resources->roomResources[0].backgroundDrawData.drawCommandCount);
    SRL::Debug::Print(1,18,"player: %d", (u8)resources->roomResources[1].backgroundDrawData.drawCommandCount);
    SRL::Debug::Print(1,19,"player: %d", (u8)resources->roomResources[2].backgroundDrawData.drawCommandCount);
    SRL::Debug::Print(1,20,"player: %d", (u8)resources->roomResources[3].backgroundDrawData.drawCommandCount);
    SRL::Debug::Print(1,21,"player: %d", (u8)resources->roomResources[4].backgroundDrawData.drawCommandCount);



    SRL::Debug::Print(1,3,"VDP2 ScrollScreen Sample");
    
    //Main Game Loop 
    while(1)
    {
        Game_Update(gameData, resources);

        SRL::Core::Synchronize();

        //move positions of NBG0 and NBG1 scrolls:
        Nbg0Position += Vector2D(1.0, 1.0);
        Nbg1Position += Vector2D(-2.0, 1.0);
        SRL::VDP2::NBG0::SetPosition(Nbg0Position);
        SRL::VDP2::NBG1::SetPosition(Nbg1Position);
    }
    return 0;
}
