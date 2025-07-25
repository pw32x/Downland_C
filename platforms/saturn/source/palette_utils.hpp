#ifndef PALETTE_UTILS_INCLUDE_H
#define PALETTE_UTILS_INCLUDE_H

#include <srl.hpp>

#include "base_types.h"

namespace PaletteUtils
{

const int g_downlandPaletteColorsCount = 256;
extern SRL::Types::HighColor g_downlandPalette[g_downlandPaletteColorsCount];

class PaletteLoader
{
public:
    // Load color palettes here
    static int16_t LoadDownlandPalette(SRL::Bitmap::BitmapInfo* bitmap)
    {
        static int16_t g_downlandPaletteIndex = -1;

        if (g_downlandPaletteIndex != -1)
            return g_downlandPaletteIndex;

        // Get free CRAM bank
        int32_t id = SRL::CRAM::GetFreeBank(bitmap->ColorMode);

        if (id >= 0)
        {
            g_downlandPaletteIndex = id;

            SRL::CRAM::Palette cramPalette(bitmap->ColorMode, id);

            if (cramPalette.Load((SRL::Types::HighColor*)bitmap->Palette->Colors, 
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

    static int16_t Load(SRL::Bitmap::BitmapInfo* bitmap)
    {
        // Get free CRAM bank
        int32_t id = SRL::CRAM::GetFreeBank(bitmap->ColorMode);

        if (id >= 0)
        {
            SRL::CRAM::Palette cramPalette(bitmap->ColorMode, id);

            if (cramPalette.Load((SRL::Types::HighColor*)bitmap->Palette->Colors, 
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

};
}

#endif
