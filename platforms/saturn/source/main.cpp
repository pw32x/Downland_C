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

// Load color palettes here
int16_t LoadPalette(SRL::Bitmap::BitmapInfo* bitmap)
{
    // Get free CRAM bank
    int32_t id = SRL::CRAM::GetFreeBank(bitmap->ColorMode);

    if (id >= 0)
    {
        SRL::CRAM::Palette cramPalette(bitmap->ColorMode, id);

        if (cramPalette.Load((HighColor*)bitmap->Palette->Colors, 
                             bitmap->Palette->Count) >= 0)
        {
            // Mark bank as in use
            SRL::CRAM::SetBankUsedState(id, bitmap->ColorMode, true);
            return id;
        }

        return id;
    }

    // No free bank found
    return -1;
}

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define HALF_SCREEN_WIDTH (SCREEN_WIDTH / 2)
#define HALF_SCREEN_HEIGHT (SCREEN_HEIGHT / 2)

void setupQuad(SRL::Math::Types::Fxp x, 
               SRL::Math::Types::Fxp y, 
               SRL::Math::Types::Fxp width, 
               SRL::Math::Types::Fxp height, 
               SRL::Math::Types::Vector2D points[4])
{
    width -= 1;
    height -= 1;
    x -= HALF_SCREEN_WIDTH;
    y -= HALF_SCREEN_HEIGHT;

    points[0].X = x;
    points[0].Y = y;
    points[1].X = x + width;
    points[1].Y = y;
    points[2].X = x + width;
    points[2].Y = y + height;
    points[3].X = x;
    points[3].Y = y + height;
}

class Sprite
{
public:
    int16_t m_textureIndex;
    int16_t m_x;
    int16_t m_y;
    int16_t m_width;
    int16_t m_height;

public:
    Sprite(int16_t x, 
           int16_t y, 
           SRL::Bitmap::IBitmap* bitmap) 
        : m_textureIndex(-1), 
          m_x(x), 
          m_y(y),
          m_width(0),
          m_height(0)
    {
        m_textureIndex = SRL::VDP1::TryLoadTexture(bitmap, LoadPalette);

        m_width = bitmap->GetInfo().Width;
        m_height = bitmap->GetInfo().Height;
    }

    Sprite(int16_t _x, int16_t _y, const char* filename) 
        : m_textureIndex(-1), 
          m_x(_x), 
          m_y(_y),
          m_width(0),
          m_height(0)
    {
        // Load texture
        SRL::Bitmap::TGA *tga = new SRL::Bitmap::TGA(filename); // Loads TGA file into main RAM

        m_textureIndex = SRL::VDP1::TryLoadTexture(tga, LoadPalette);    // Loads TGA into VDP1

        m_width = tga->GetInfo().Width;
        m_height = tga->GetInfo().Height;

        delete tga;
    }

    void Draw()
    {
        static SRL::Math::Types::Vector2D points[4];

        setupQuad(m_x, 
                  m_y, 
                  m_width, 
                  m_height, 
                  points);

        // Simple sprite
        SRL::Scene2D::DrawSprite(m_textureIndex, points, 500);
    }
};

class MyBitmap : public SRL::Bitmap::IBitmap
{
public:
    MyBitmap(u8* bitmapData, 
             int width, 
             int height, 
             SRL::Types::HighColor* paletteColors, 
             size_t numColors) 
        : m_bitmapData(bitmapData),
          m_palette(paletteColors, numColors),
          m_bitmapInfo(width, height, &m_palette)
    {

    }

    virtual uint8_t* GetData()
    {
        return m_bitmapData;
    }
        
    /** @brief Get bitmap info
        * @return Bitmap info
        */
    virtual SRL::Bitmap::BitmapInfo GetInfo()
    {
        return m_bitmapInfo;
    }

public:
    u8* m_bitmapData;
    SRL::Bitmap::Palette m_palette;
    SRL::Bitmap::BitmapInfo m_bitmapInfo;
};

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

    const int numColors = 256;
    SRL::Types::HighColor customPalette[numColors] =
    {
        // black, blue, orange, white
        0x0000, 0x001F, 0xFC80, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    };

    MyBitmap myBitmap(customSprite, 16, 16, customPalette, numColors);

    Sprite mySpriteCustom(160, 40, &myBitmap);

    Sprite mySprite1(0, 40, "TEST.TGA");
    Sprite mySprite2(80, 120, "TEST8BPP.TGA");
    Sprite mySprite3(160, 160, "TEST4BPP.TGA");


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

    SRL::Debug::Print(1,13,"ColorMode %d", myBitmap.m_bitmapInfo.ColorMode);
    SRL::Debug::Print(1,14,"Color count %d", myBitmap.m_bitmapInfo.Palette->Count);
    SRL::Debug::Print(1,22,"Color count 2 %d", myBitmap.m_bitmapInfo.colorCount);
    

    //SRL::Debug::Print(1,13,"GameData size %d", sizeof(GameData));
    //SRL::Debug::Print(1,14,"Resources size %d", sizeof(Resources));

    //SRL::Debug::Print(1,17,"player: %d", (u8)resources->roomResources[0].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,18,"player: %d", (u8)resources->roomResources[1].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,19,"player: %d", (u8)resources->roomResources[2].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,20,"player: %d", (u8)resources->roomResources[3].backgroundDrawData.drawCommandCount);
    //SRL::Debug::Print(1,21,"player: %d", (u8)resources->roomResources[4].backgroundDrawData.drawCommandCount);

    while(1)
    {
        Game_Update(gameData, resources);

        SRL::Core::Synchronize();

        mySprite1.Draw();
        mySprite2.Draw();
        mySprite3.Draw();
        mySpriteCustom.Draw();

        //move positions of NBG0 and NBG1 scrolls:
        Nbg0Position += Vector2D(1.0, 1.0);
        Nbg1Position += Vector2D(-2.0, 1.0);
        SRL::VDP2::NBG0::SetPosition(Nbg0Position);
        SRL::VDP2::NBG1::SetPosition(Nbg1Position);
    }
    return 0;
}
