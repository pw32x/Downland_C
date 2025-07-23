#include "image_utils.h"

enum CrtColor
{
    CrtColor_Blue,
    CrtColor_Orange
};

/*
void convert1bppImageTo8bppCrtEffectImageWithNewWidth(const dl_u8* originalImage,
                                                      dl_u8* destinationImage,
                                                      dl_u16 width,
                                                      dl_u16 height) 
{
    const dl_u8 bytesPerRow = width / 8;

    // Color definitions
    const dl_u8 BLACK  = 0x00; // 00 black
    const dl_u8 BLUE   = 0x1; // 01 blue
    const dl_u8 ORANGE = 0x2; // 10 orange
    const dl_u8 WHITE  = 0x3; // 11 white

    for (int y = 0; y < height; ++y) 
    {
        // every pair of bits generates a color for the two corresponding
        // pixels of the destination texture, so:
        // source bits:        00 01 10 11
        // final pixel colors: black, black, blue, blue, orange, orange, white, white.
        dl_u32 yOffset = y * 320;

        for (int x = 0; x < width; x += 2) 
        {
            int byteIndex = (y * bytesPerRow) + (x / 8);
            int bitOffset = 7 - (x % 8);

            // Read two adjacent bits
            dl_u8 bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
            dl_u8 bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

            // Determine base color
            dl_u8 color = BLACK;
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
            dl_u8 leftPixel = destinationImage[yOffset + x];
            dl_u8 rightPixel = destinationImage[yOffset + x + 1];

            if (rightPixel == BLUE && x < width - 2)
            {
                dl_u8 pixel3 = destinationImage[yOffset + x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    rightPixel = WHITE;
                    leftPixel = BLACK;
                }
            }
            else if (leftPixel == ORANGE && x >= 2)
            {
                dl_u8 pixel0 = destinationImage[yOffset + x - 1];
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
*/

/*
void convert1bppImageTo8bppCrtEffectImageWithNewWidth(const dl_u8* originalImage,
                                                      dl_u8* destinationImage,
                                                      dl_u16 width,
                                                      dl_u16 height) 
{
    const dl_u8 bytesPerRow = width / 8;

    const dl_u8 BLACK  = 0x00;
    const dl_u8 BLUE   = 0x1;
    const dl_u8 ORANGE = 0x2;
    const dl_u8 WHITE  = 0x3;

    dl_u8 rowBuffer[320];

    for (int y = 0; y < height; ++y) 
    {
        int yOffset = y * 320;

        // First pass: decode bits into colors (2 output pixels per bit pair)
        for (int x = 0; x < width; x += 2) 
        {
            int byteIndex = (y * bytesPerRow) + (x / 8);
            int bitOffset = 7 - (x % 8);

            dl_u8 bits = originalImage[byteIndex];
            dl_u8 bit1 = (bits >> bitOffset) & 1;
            dl_u8 bit2 = (bits >> (bitOffset - 1)) & 1;

            dl_u8 color = BLACK;
            if (bit1 == 0 && bit2 == 1) color = BLUE;
            else if (bit1 == 1 && bit2 == 0) color = ORANGE;
            else if (bit1 == 1 && bit2 == 1) color = WHITE;

            rowBuffer[x]     = color;
            rowBuffer[x + 1] = color;
        }

        // Second pass: apply CRT artifact logic in-place in rowBuffer
        for (int x = 0; x < width; x += 2) 
        {
            dl_u8 leftPixel  = rowBuffer[x];
            dl_u8 rightPixel = rowBuffer[x + 1];

            if (rightPixel == BLUE && x < width - 2)
            {
                dl_u8 pixel3 = rowBuffer[x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    rightPixel = WHITE;
                    leftPixel = BLACK;
                }
            }
            else if (leftPixel == ORANGE && x >= 2)
            {
                dl_u8 pixel0 = rowBuffer[x - 1];
                if (pixel0 == BLUE || pixel0 == WHITE)
                {
                    leftPixel = WHITE;
                    rightPixel = BLACK;
                }
            }

            rowBuffer[x]     = leftPixel;
            rowBuffer[x + 1] = rightPixel;
        }

        // Final pass: copy modified row to destination
        for (int x = 0; x < width; ++x) 
        {
            destinationImage[yOffset + x] = rowBuffer[x];
        }
    }
}
*/

/*
void convert1bppImageTo8bppCrtEffectImageWithNewWidth(const dl_u8* originalImage,
                                                      dl_u8* destinationImage,
                                                      dl_u16 width,
                                                      dl_u16 height) 
{
    const dl_u8 bytesPerRow = width / 8;

    const dl_u8 BLACK  = 0x04;
    const dl_u8 BLUE   = 0x01;
    const dl_u8 ORANGE = 0x02;
    const dl_u8 WHITE  = 0x03;

    dl_u8 rowBuffer[320];  // Temp buffer for one scanline (max width = 320 assumed)

    for (int y = 0; y < height; ++y) 
    {
        int yOffset = y * 320;

        // Decode 1bpp input into doubled pixels in rowBuffer
        for (int x = 0; x < width; x += 2) 
        {
            int byteIndex = (y * bytesPerRow) + (x / 8);
            int bitOffset = 7 - (x % 8);

            dl_u8 bits = originalImage[byteIndex];
            dl_u8 bit1 = (bits >> bitOffset) & 1;
            dl_u8 bit2 = (bits >> (bitOffset - 1)) & 1;

            dl_u8 color = BLACK;
            if (bit1 == 0 && bit2 == 1) color = BLUE;
            else if (bit1 == 1 && bit2 == 0) color = ORANGE;
            else if (bit1 == 1 && bit2 == 1) color = WHITE;

            rowBuffer[x]     = color;
            rowBuffer[x + 1] = color;
        }

        // Apply CRT artifact effect in the temp buffer
        for (int x = 0; x < width; x += 2) 
        {
            dl_u8 left  = rowBuffer[x];
            dl_u8 right = rowBuffer[x + 1];

            if (right == BLUE) 
            {
                dl_u8 pixel3 = rowBuffer[x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE) 
                {
                    left  = BLACK;
                    right = WHITE;
                }
            }
            else if (left == ORANGE) 
            {
                dl_u8 pixel0 = rowBuffer[x - 1];
                if (pixel0 == BLUE || pixel0 == WHITE) 
                {
                    left  = WHITE;
                    right = BLACK;
                }
            }

            rowBuffer[x]     = left;
            rowBuffer[x + 1] = right;
        }

        // Write final row to framebuffer with 16-bit big-endian stores
        dl_u16* destRow = (dl_u16*)(destinationImage + yOffset);
        for (int x = 0; x < width; x += 2) 
        {
            destRow[x / 2] = (rowBuffer[x] << 8) | rowBuffer[x + 1];
        }
    }
}
*/
void convert1bppFramebufferTo8bppCrtEffectFramebuffer(const dl_u8* originalImage,
                                                      dl_u8* destinationImage,
                                                      dl_u16 width,
                                                      dl_u16 height)
{
    const dl_u8 bytesPerRow = width / 8;

    const dl_u8 BLACK  = 0x04;
    const dl_u8 BLUE   = 0x01;
    const dl_u8 ORANGE = 0x02;
    const dl_u8 WHITE  = 0x03;

    dl_u8 rowBuffer[320];

    // Lookup for 2-bit pairs ? color
    // bits: 00=BLACK, 01=BLUE, 10=ORANGE, 11=WHITE
    const dl_u8 pairToColor[4] = { BLACK, BLUE, ORANGE, WHITE };

    for (int y = 0; y < height; ++y)
    {
        int yOffset = y * 320;

        // Decode + CRT effect in one pass
        // Process per byte of input (8 bits = 4 pairs)
        for (int byteX = 0; byteX < bytesPerRow; ++byteX)
        {
            dl_u8 bits = originalImage[y * bytesPerRow + byteX];

            // Extract four 2-bit pairs, MSB first
            // Pair 0: bits 7,6
            // Pair 1: bits 5,4
            // Pair 2: bits 3,2
            // Pair 3: bits 1,0

            dl_u8 pairs[4];
            pairs[0] = (bits >> 6) & 0x3;
            pairs[1] = (bits >> 4) & 0x3;
            pairs[2] = (bits >> 2) & 0x3;
            pairs[3] = (bits >> 0) & 0x3;

            // Decode colors (each pair repeated twice)
            int baseX = byteX * 8;
            for (int i = 0; i < 4; ++i)
            {
                dl_u8 color = pairToColor[pairs[i]];
                rowBuffer[baseX + i * 2]     = color;
                rowBuffer[baseX + i * 2 + 1] = color;
            }
        }

        // Apply CRT artifact effect in one pass
        // Note: safe to process full width - 2 because we check bounds in condition
        for (int x = 0; x < width; x += 2)
        {
            dl_u8 left  = rowBuffer[x];
            dl_u8 right = rowBuffer[x + 1];

            if (right == BLUE && x < width - 2)
            {
                dl_u8 pixel3 = rowBuffer[x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    left  = BLACK;
                    right = WHITE;
                }
            }
            else if (left == ORANGE && x >= 2)
            {
                dl_u8 pixel0 = rowBuffer[x - 1];
                if (pixel0 == BLUE || pixel0 == WHITE)
                {
                    left  = WHITE;
                    right = BLACK;
                }
            }

            rowBuffer[x]     = left;
            rowBuffer[x + 1] = right;
        }

        // Write output with 16-bit stores
        dl_u16* destRow = (dl_u16*)(destinationImage + yOffset);
        for (int x = 0; x < width; x += 2)
        {
            destRow[x / 2] = (rowBuffer[x] << 8) | rowBuffer[x + 1];
        }
    }
}
