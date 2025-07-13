#include "image_utils.h"

void convert1bppImageTo8bppCrtEffectImage(const dl_u8* originalImage,
                                          dl_u8* destinationImage,
                                          dl_u16 width,
                                          dl_u16 height,
                                          enum CrtColor crtColor) 
{
    const dl_u8 bytesPerRow = width / 8;

    // Color definitions
    const dl_u8 BLACK  = 0x00; // 00 black
    const dl_u8 BLUE   = crtColor == CrtColor_Blue ? 0x1 : 0x2; // 01 blue
    const dl_u8 ORANGE = crtColor == CrtColor_Blue ? 0x2 : 0x1; // 10 orange
    const dl_u8 WHITE  = 0x3; // 11 white

    dl_u32 yOffset;
    int x;
    int y;
    int byteIndex;
    int bitOffset;
    dl_u8 bit1;
    dl_u8 bit2;
    dl_u8 color;
    dl_u8 leftPixel;
    dl_u8 rightPixel;
    dl_u8 pixel3;
    dl_u8 pixel0;

    for (y = 0; y < height; ++y) 
    {
        // every pair of bits generates a color for the two corresponding
        // pixels of the destination texture, so:
        // source bits:        00 01 10 11
        // final pixel colors: black, black, blue, blue, orange, orange, white, white.
        yOffset = y * width;

        for (x = 0; x < width; x += 2) 
        {
            byteIndex = (y * bytesPerRow) + (x / 8);
            bitOffset = 7 - (x % 8);

            // Read two adjacent bits
            bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
            bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

            // Determine base color
            color = BLACK;
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
        for (x = 0; x < width; x += 2) 
        {
            leftPixel = destinationImage[yOffset + x];
            rightPixel = destinationImage[yOffset + x + 1];

            if (rightPixel == BLUE && x < width - 2)
            {
                pixel3 = destinationImage[yOffset + x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    rightPixel = WHITE;
                    leftPixel = BLACK;
                }
            }
            else if (leftPixel == ORANGE && x >= 2)
            {
                pixel0 = destinationImage[yOffset + x - 1];
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


