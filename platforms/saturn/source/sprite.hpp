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


class Sprite
{
public:
    Sprite(const u8* originalSprite, 
           s16 width, 
           s16 height, 
           u8 numFrames)
        : m_originalSprite(originalSprite),
          m_width(width),
          m_height(height),
          m_numFrames(numFrames),
          m_frameTextureIndexes(NULL)
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

    void draw(s16 x, s16 y, s16 frameNumber)
    {
        SRL::Math::Types::Vector2D points[4];

        GeometryHelpers::Quad::setup(x, 
                                     y, 
                                     m_width, 
                                     m_height, 
                                     points);

        // Simple sprite
        SRL::Scene2D::DrawSprite(m_frameTextureIndexes[frameNumber], points, 500);
    }

public:

	s16 m_width;
	s16 m_height;
	u8 m_numFrames;
    s16* m_frameTextureIndexes;
	const u8* m_originalSprite;
};

class RegenSprite
{
public:
    RegenSprite(const u8* originalSprite, 
                s16 width, 
                s16 height, 
                u8 numFrames)
        : m_originalSprite(originalSprite),
          m_width(width),
          m_height(height),
          m_numFrames(numFrames),
          m_frameTextureIndexes(NULL)
    {
        // pre-generate a number of regeneration sprites ahead of time
        // instead of creating them on the fly.    

        u8 totalFrames = numFrames * 2;

        m_frameTextureIndexes = new s16[totalFrames];

        u8* buffer = new u8[width * height];
        u8* regenBuffer = new u8[(width / 8) * height];

        // right side
        for (int loop = 0; loop < numFrames; loop++)
        {
            memset(regenBuffer, 0, (width / 8) * height);
            drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													        m_height,
													        (u8*)regenBuffer);

            ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(regenBuffer,
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

        }

        originalSprite += ((width / 8) * height) * 6; // PLAYER_SPRITE_LEFT_STAND

        // left size
        for (int loop = 0; loop < numFrames; loop++)
        {
            memset(regenBuffer, 0, (width / 8) * height);
            drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													        m_height,
													        (u8*)regenBuffer);

            ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(regenBuffer,
                                                                             buffer,
                                                                             width,
                                                                             height,
                                                                             ImageUtils::ImageConverter::CrtColor::Blue);

            BitmapUtils::InMemoryBitmap inMemoryBitmap(buffer, 
                                                       width, 
                                                       height, 
                                                       PaletteUtils::g_downlandPalette, 
                                                       PaletteUtils::g_downlandPaletteColorsCount);

            m_frameTextureIndexes[loop + numFrames] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                                                PaletteUtils::PaletteLoader::LoadDownlandPalette);

        }

        delete [] buffer;
        delete [] regenBuffer;
    }
        
    void draw(s16 x, s16 y, s16 frameNumber)
    {
        SRL::Math::Types::Vector2D points[4];

        GeometryHelpers::Quad::setup(x, 
                                     y, 
                                     m_width, 
                                     m_height, 
                                     points);

        // Simple sprite
        SRL::Scene2D::DrawSprite(m_frameTextureIndexes[frameNumber], points, 500);
    }

public:

	s16 m_width;
	s16 m_height;
	u8 m_numFrames;
    s16* m_frameTextureIndexes;
	const u8* m_originalSprite;
};


class SplatSprite
{
public:
    SplatSprite(const u8* originalSprite, 
                s16 width, 
                s16 height)
        : m_originalSprite(originalSprite),
          m_width(width),
          m_height(height),
          m_numFrames(2),
          m_frameTextureIndexes(NULL)
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
        
    void draw(s16 x, s16 y, s16 frameNumber)
    {
        SRL::Math::Types::Vector2D points[4];

        GeometryHelpers::Quad::setup(x, 
                                     y, 
                                     m_width, 
                                     m_height, 
                                     points);

        // Simple sprite
        SRL::Scene2D::DrawSprite(m_frameTextureIndexes[frameNumber], points, 500);
    }

public:

	s16 m_width;
	s16 m_height;
	u8 m_numFrames;
    s16* m_frameTextureIndexes;
	const u8* m_originalSprite;
};


#endif