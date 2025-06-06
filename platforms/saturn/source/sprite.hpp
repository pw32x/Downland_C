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

        ImageUtils::ImageConverter::convert1bppImageTo8bppCrtEffectImage(originalSprite,
                                                                         buffer,
                                                                         width,
                                                                         height,
                                                                         ImageUtils::ImageConverter::CrtColor::Blue);

        for (int loop = 0; loop < numFrames; loop++)
        {
            BitmapUtils::InMemoryBitmap inMemoryBitmap(buffer, 
                                                       width, 
                                                       height, 
                                                       PaletteUtils::g_downlandPalette, 
                                                       PaletteUtils::g_downlandPaletteColorsCount);

            m_frameTextureIndexes[loop] = SRL::VDP1::TryLoadTexture(&inMemoryBitmap, 
                                                                    PaletteUtils::PaletteLoader::LoadDownlandPalette);
        }

        delete [] buffer;
    }

    void updateSprite(u8 frameNumber, const u8* originalSprite)
    {
        /*
        std::vector<u32>& spriteFrame = m_frames[frameNumber];

        SDLUtils_convert1bppImageTo32bppCrtEffectImage(originalSprite,
                                                       spriteFrame.data(),
                                                       m_width,
                                                       m_height,
                                                       CrtColor::Blue);
        */
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