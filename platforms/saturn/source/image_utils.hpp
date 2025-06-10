#ifndef UTILS_INCLUDE_H
#define UTILS_INCLUDE_H

#include "base_types.h"

namespace ImageUtils
{
class ImageConverter
{
public:
    enum CrtColor
    {
        Blue,
        Orange
    };

public:
    static void convert1bppImageTo8bppCrtEffectImage(const u8* originalImage,
                                                     u8* destinationImage,
                                                     u16 width,
                                                     u16 height,
                                                     CrtColor crtColor) 
    {
        const u8 bytesPerRow = width / 8;

        // Color definitions
        const u8 BLACK  = 0x0000; // 00 black
        const u8 BLUE   = crtColor == CrtColor::Blue ? 0x1 : 0x2; // 01 blue
        const u8 ORANGE = crtColor == CrtColor::Blue ? 0x2 : 0x1; // 10 orange
        const u8 WHITE  = 0x3; // 11 white

        for (int y = 0; y < height; ++y) 
        {
            // every pair of bits generates a color for the two corresponding
            // pixels of the destination texture, so:
            // source bits:        00 01 10 11
            // final pixel colors: black, black, blue, blue, orange, orange, white, white.
            u32 yOffset = y * width;

            for (int x = 0; x < width; x += 2) 
            {
                int byteIndex = (y * bytesPerRow) + (x / 8);
                int bitOffset = 7 - (x % 8);

                // Read two adjacent bits
                u8 bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
                u8 bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

                // Determine base color
                u8 color = BLACK;
                if (bit1 == 0 && bit2 == 1) color = BLUE;
                else if (bit1 == 1 && bit2 == 0) color = ORANGE;
                else if (bit1 == 1 && bit2 == 1) color = WHITE;

                // Apply base color
                destinationImage[yOffset + x]     = color;
                destinationImage[yOffset + x + 1] = color;
            }

            // Apply a quick and dirty crt artifact effect
            // pixels whose original bits are adjacent are converted to white
            // source colors: black, black, blue, blue, orange, orange, white, white.
            // source bits:  00 01 10 11
            // seen as:      00 00 01 01 10 10 11 11 // forth and fifth pairs have adjacent bits. 
            //                                          Turn both corresponding pixels to white.
            //                                          Also turn off the other pixel in the pair to
            //                                          black.
            // final final:  black, black, black, white, white, black, white, white
            for (int x = 0; x < width; x += 2) 
            {
                u8 leftPixel = destinationImage[yOffset + x];
                u8 rightPixel = destinationImage[yOffset + x + 1];

                if (rightPixel == BLUE && x < width - 2)
                {
                    u8 pixel3 = destinationImage[yOffset + x + 2];
                    if (pixel3 == ORANGE || pixel3 == WHITE)
                    {
                        rightPixel = WHITE;
                        leftPixel = BLACK;
                    }
                }
                else if (leftPixel == ORANGE && x >= 2)
                {
                    u8 pixel0 = destinationImage[yOffset + x - 1];
                    if (pixel0 == BLUE || pixel0 == WHITE)
                    {
                        leftPixel = WHITE;
                        rightPixel = BLACK;
                    }
                }

                destinationImage[yOffset + x] = leftPixel;
                destinationImage[yOffset + x + 1] = rightPixel;
            }
        }
    }

    static void convert1bppImageTo8bppBlue(const u8* originalImage,
                                           u8* destinationImage,
                                           u16 width,
                                           u16 height) 
    {
        const u8 BLACK  = 0x0; // 00 black
        const u8 BLUE   = 0x1; // 01 blue
        const u8 ORANGE = 0x2; // 10 orange
        const u8 WHITE  = 0x3; // 11 white

        const u8 bytesPerRow = width / 8;

        for (int y = 0; y < height; ++y) 
        {
            u32 yOffset = y * width;

            for (int x = 0; x < width; x += 2) 
            {
                int byteIndex = (y * bytesPerRow) + (x / 8);
                int bitOffset = 7 - (x % 8);

                // Read two adjacent bits
                u8 bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
                u8 bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

                // Determine base color
                u8 color = BLACK;
                if (bit1 == 0 && bit2 == 1) color = BLUE;
                else if (bit1 == 1 && bit2 == 0) color = ORANGE;
                else if (bit1 == 1 && bit2 == 1) color = 1; // don't care. just set to blue.

                // Apply base color
                destinationImage[yOffset + x]     = color;
                destinationImage[yOffset + x + 1] = color;
            }
        }
    }
};
}
#endif