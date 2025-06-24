#ifndef BITMAP_UTILS_INCLUDE_H
#define BITMAP_UTILS_INCLUDE_H

#include <srl.hpp>

#include "base_types.h"

namespace BitmapUtils
{

class InMemoryBitmap : public SRL::Bitmap::IBitmap
{
public:
    InMemoryBitmap(dl_u8* bitmapData, 
                   dl_u16 width, 
                   dl_u16 height, 
                   SRL::Types::HighColor* paletteColors, 
                   size_t numColors) 
        : m_bitmapData(bitmapData),
          m_palette(paletteColors, numColors),
          m_bitmapInfo(width, height, &m_palette)
    {

    }

    virtual dl_u8* GetData()
    {
        return m_bitmapData;
    }
        
    virtual SRL::Bitmap::BitmapInfo GetInfo()
    {
        return m_bitmapInfo;
    }

public:
    dl_u8* m_bitmapData;
    SRL::Bitmap::Palette m_palette;
    SRL::Bitmap::BitmapInfo m_bitmapInfo;
};
}

#endif
