#include <srl.hpp>

extern "C"
{
#include "base_types.h"
#include "resource_types.h"
#include "sound.h"
}

#include "game_runner.hpp"


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


class GameObject
{
public:
    int16_t m_textureIndex;
    int16_t m_x;
    int16_t m_y;
    int16_t m_width;
    int16_t m_height;

public:
    GameObject(int16_t x, 
               int16_t y, 
               SRL::Bitmap::IBitmap* bitmap) 
        : m_textureIndex(-1), 
          m_x(x), 
          m_y(y),
          m_width(0),
          m_height(0)
    {
        m_textureIndex = SRL::VDP1::TryLoadTexture(bitmap, PaletteUtils::PaletteLoader::Load);

        m_width = bitmap->GetInfo().Width;
        m_height = bitmap->GetInfo().Height;
    }

    GameObject(int16_t _x, int16_t _y, const char* filename) 
        : m_textureIndex(-1), 
          m_x(_x), 
          m_y(_y),
          m_width(0),
          m_height(0)
    {
        // Load texture
        SRL::Bitmap::TGA *tga = new SRL::Bitmap::TGA(filename); // Loads TGA file into main RAM

        m_textureIndex = SRL::VDP1::TryLoadTexture(tga, PaletteUtils::PaletteLoader::Load);    // Loads TGA into VDP1

        m_width = tga->GetInfo().Width;
        m_height = tga->GetInfo().Height;

        delete tga;
    }

    void Draw()
    {
        static SRL::Math::Types::Vector2D points[4];

        GeometryHelpers::Quad::setup(m_x, 
                                     m_y, 
                                     m_width, 
                                     m_height, 
                                     points);

        // Simple sprite
        SRL::Scene2D::DrawSprite(m_textureIndex, points, 500);
    }
};





Resources* g_resources;
GameRunner* g_gameRunner;

int main()
{
    SRL::Core::Initialize(HighColor(20,10,50));
    Digital port0(0); // Initialize gamepad on port 0
  

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

    u8 customSprite[16*16] = 
    {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    u8 playerObjectBuffer[16*16];
    ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(g_resources->sprites_player,
                                                                     playerObjectBuffer,
                                                                     16,
                                                                     16,
                                                                     ImageUtils::ImageConverter::CrtColor::Blue);



    BitmapUtils::InMemoryBitmap myBitmap(customSprite, 
                                         16, 
                                         16, 
                                         PaletteUtils::g_downlandPalette, 
                                         PaletteUtils::g_downlandPaletteColorsCount);

    BitmapUtils::InMemoryBitmap myBitmap2(playerObjectBuffer, 
                                          16, 
                                          16, 
                                          PaletteUtils::g_downlandPalette, 
                                          PaletteUtils::g_downlandPaletteColorsCount);

    GameObject myCustomObject(160, 40, &myBitmap);
    GameObject playerObject(220, 40, &myBitmap2);

    GameObject myObject1(0, 40, "TEST.TGA");
    GameObject myObject2(80, 120, "TEST8BPP.TGA");
    GameObject myObject3(160, 160, "TEST4BPP.TGA");


    /*
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
        g_gameRunner->update();

        SRL::Core::Synchronize();

        const Drop* drop = &g_gameRunner->m_gameData.dropData.drops[0];

        //SRL::Debug::Print(1,17,"x %02d", drop->x << 1);
	    //SRL::Debug::Print(1,18,"y %02d", drop->y >> 8);

        //myObject1.m_x = drop->x << 1;
        //myObject1.m_y = drop->y >> 8;

        g_gameRunner->draw();

        //myObject1.Draw();
        //myObject2.Draw();
        //myObject3.Draw();
        myCustomObject.Draw();
        playerObject.Draw();

        //move positions of NBG0 and NBG1 scrolls:
        //Nbg0Position += Vector2D(1.0, 1.0);
        //Nbg1Position += Vector2D(-2.0, 1.0);
        //SRL::VDP2::NBG0::SetPosition(Nbg0Position);
        //SRL::VDP2::NBG1::SetPosition(Nbg1Position);
    }
    return 0;
}
