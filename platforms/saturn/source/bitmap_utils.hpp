#ifndef BITMAP_UTILS_INCLUDE_H
#define BITMAP_UTILS_INCLUDE_H

#include <srl.hpp>

#include "base_types.h"

namespace BitmapUtils
{

class InMemoryBitmap : public SRL::Bitmap::IBitmap
{
public:
    InMemoryBitmap(u8* bitmapData, 
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
        
    virtual SRL::Bitmap::BitmapInfo GetInfo()
    {
        return m_bitmapInfo;
    }

public:
    u8* m_bitmapData;
    SRL::Bitmap::Palette m_palette;
    SRL::Bitmap::BitmapInfo m_bitmapInfo;
};
}

#endif
