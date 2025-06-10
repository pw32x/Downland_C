#ifndef SPRITE_INCLUDE_H
#define SPRITE_INCLUDE_H

#include <srl.hpp>

#include "palette_utils.hpp"
#include "bitmap_utils.hpp"
#include "geometry_helpers.hpp"
#include "image_utils.hpp"

extern "C"
{
#include "base_types.h"
#include "alloc.h"
#include "draw_utils.h"
}

class SpriteBase 
{
public:
    SpriteBase(const u8* originalSprite, 
               s16 width, 
               s16 height, 
               u8 numFrames)
        : m_originalSprite(originalSprite),
          m_width(width),
          m_height(height),
          m_numFrames(numFrames),
          m_frameTextureIndexes(NULL)
    {

    }

    void draw(s16 x, s16 y, s16 frameNumber)
    {
        SRL::Math::Types::Vector2D points[4];

        GeometryHelpers::Quad::setup(x + SCREEN_OFFSET_X, 
                                     y + SCREEN_OFFSET_Y, 
                                     m_width, 
                                     m_height, 
                                     points);

        // Simple sprite
        SRL::Scene2D::DrawSprite(m_frameTextureIndexes[frameNumber], points, 500);
    }

    u8 getNumFrames() { return m_numFrames; }

protected:
	s16 m_width;
	s16 m_height;
	u8 m_numFrames;
    s16* m_frameTextureIndexes;
	const u8* m_originalSprite;
};

class Sprite : public SpriteBase
{
public:
    Sprite(const u8* originalSprite, 
           s16 width, 
           s16 height, 
           u8 numFrames)
        : SpriteBase(originalSprite, width, height, numFrames)
    {
        m_frameTextureIndexes = new s16[numFrames];

        u8* buffer = new u8[width * height];

        const u8* spriteRunner = originalSprite;

        for (int loop = 0; loop < numFrames; loop++)
        {
            ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(spriteRunner,
                                                                             buffer,
                                                                             width,
                                                                             height,
                                                                             ImageUtils::ImageConverter::CrtColor::Blue);

            BitmapUtils::InMemoryBitmap inMemoryBitmap(buffer, 
                                                       width, 
                                                       height, 
                                                       PaletteUtils::g_downlandPalette, 
                                                       PaletteUtils::g_downlandPaletteColorsCount);

            m_frameTextureIndexes[loop] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                                    PaletteUtils::PaletteLoader::LoadDownlandPalette);

            spriteRunner += (width / 8) * height;
        }

        delete [] buffer;
    }
};

class RegenSprite : public SpriteBase
{
public:
    RegenSprite(const u8* originalSprite, 
                s16 width, 
                s16 height, 
                s16 clippedHeight,
                u8 numFrames)
        : SpriteBase(originalSprite, width, height, numFrames)
    {
        // pre-generate a number of regeneration sprites ahead of time
        // instead of creating them on the fly.    

        u8 totalFrames = numFrames * 2;

        m_frameTextureIndexes = new s16[totalFrames];

        u8* buffer = new u8[width * clippedHeight];
        u8* regenBuffer = new u8[(width / 8) * clippedHeight];

        // right side
        for (int loop = 0; loop < numFrames; loop++)
        {
            memset(regenBuffer, 0, (width / 8) * clippedHeight);
            drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													        clippedHeight,
													        (u8*)regenBuffer);

            ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(regenBuffer,
                                                                             buffer,
                                                                             width,
                                                                             clippedHeight,
                                                                             ImageUtils::ImageConverter::CrtColor::Blue);


            BitmapUtils::InMemoryBitmap inMemoryBitmap(buffer, 
                                                       width, 
                                                       clippedHeight, 
                                                       PaletteUtils::g_downlandPalette, 
                                                       PaletteUtils::g_downlandPaletteColorsCount);

            m_frameTextureIndexes[loop] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                                    PaletteUtils::PaletteLoader::LoadDownlandPalette);

        }

        originalSprite += ((width / 8) * height) * 6; // PLAYER_SPRITE_LEFT_STAND

        // facing left
        for (int loop = 0; loop < numFrames; loop++)
        {
            memset(regenBuffer, 0, (width / 8) * clippedHeight);
            drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													        clippedHeight,
													        (u8*)regenBuffer);

            ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(regenBuffer,
                                                                             buffer,
                                                                             width,
                                                                             clippedHeight,
                                                                             ImageUtils::ImageConverter::CrtColor::Blue);

            BitmapUtils::InMemoryBitmap inMemoryBitmap(buffer, 
                                                       width, 
                                                       clippedHeight, 
                                                       PaletteUtils::g_downlandPalette, 
                                                       PaletteUtils::g_downlandPaletteColorsCount);

            m_frameTextureIndexes[loop + numFrames] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                                                PaletteUtils::PaletteLoader::LoadDownlandPalette);

        }

        m_height = clippedHeight;

        delete [] buffer;
        delete [] regenBuffer;
    }
};


class SplatSprite : public SpriteBase
{
public:
    SplatSprite(const u8* originalSprite, 
                s16 width, 
                s16 height)
        : SpriteBase(originalSprite, width, height, 2)
    {
        m_frameTextureIndexes = new s16[m_numFrames];

        u8* buffer = new u8[width * height];

        // first frame
        ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(originalSprite,
                                                                         buffer,
                                                                         width,
                                                                         height,
                                                                         ImageUtils::ImageConverter::CrtColor::Blue);


        BitmapUtils::InMemoryBitmap inMemoryBitmap(buffer, 
                                                   width, 
                                                   height, 
                                                   PaletteUtils::g_downlandPalette, 
                                                   PaletteUtils::g_downlandPaletteColorsCount);

        m_frameTextureIndexes[0] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                             PaletteUtils::PaletteLoader::LoadDownlandPalette);


        // second frame
        memset(buffer, 0, width * 5);

        m_frameTextureIndexes[1] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                             PaletteUtils::PaletteLoader::LoadDownlandPalette);

        delete [] buffer;
    }
};


class ClippedSprite : public SpriteBase
{
public:
    ClippedSprite(const u8* originalSprite, 
                  s16 width, 
                  s16 height, 
                  s16 clippedHeight,
                  u8 numFrames)
        : SpriteBase(originalSprite, width, height, numFrames)
    {
        m_frameTextureIndexes = new s16[numFrames];

        u8* buffer = new u8[width * height];

        const u8* spriteRunner = originalSprite;

        for (int loop = 0; loop < numFrames; loop++)
        {
            ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(spriteRunner,
                                                                             buffer,
                                                                             width,
                                                                             clippedHeight,
                                                                             ImageUtils::ImageConverter::CrtColor::Blue);

            BitmapUtils::InMemoryBitmap inMemoryBitmap(buffer, 
                                                       width, 
                                                       clippedHeight, 
                                                       PaletteUtils::g_downlandPalette, 
                                                       PaletteUtils::g_downlandPaletteColorsCount);

            m_frameTextureIndexes[loop] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                                    PaletteUtils::PaletteLoader::LoadDownlandPalette);

            spriteRunner += (width / 8) * height;
        }

        m_height = clippedHeight;

        delete [] buffer;
    }
};

#endif