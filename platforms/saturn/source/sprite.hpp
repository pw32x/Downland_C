#ifndef SPRITE_INCLUDE_H
#define SPRITE_INCLUDE_H

extern "C"
{
#include "base_types.h"
#include "alloc.h"
}


class Sprite
{
public:
    Sprite(const u8* originalSprite, 
           u8 width, 
           u8 height, 
           u8 numFrames)
        : m_originalSprite(originalSprite),
          m_width(width),
          m_height(height),
          m_numFrames(numFrames),
          m_frameTextureIndexes(NULL)
    {
        /*
        const u8* spriteRunner = originalSprite;

        std::vector<u32> spriteFrame;
        spriteFrame.resize(width * height);

        for (int loop = 0; loop < numFrames; loop++)
        {
            SDLUtils_convert1bppImageTo32bppCrtEffectImage(spriteRunner,
                                                           spriteFrame.data(),
                                                           width,
                                                           height,
                                                           CrtColor::Blue);

            m_frames.push_back(spriteFrame);

            spriteRunner += (width / 8) * height;
        }
        */
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

public:

	u8 m_width;
	u8 m_height;
	u8 m_numFrames;
    s16* m_frameTextureIndexes;
	const u8* m_originalSprite;
};



#endif