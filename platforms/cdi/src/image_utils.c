#include "image_utils.h"

void convert1bppImageTo8bppCrtEffect(const dl_u8* originalImage,
                                           dl_u8* destinationImage,
                                           dl_u16 width,
                                           dl_u16 height,
                                           dl_u16 destinationWidth,
                                           dl_u8 blackIndex)
{
    #define BLACK  blackIndex
    #define BLUE   0x01
    #define ORANGE 0x02
    #define WHITE  0x03

    dl_u8 rowBuffer[320];
    const dl_u8 bytesPerRow = width / 8;
    dl_u8 pairToColor[4];
    dl_u8 pairs[4];
    int x;
    int y;
    int yOffset;
    int byteX;
    dl_u8 bits;

    int baseX;
    int i;

    dl_u8 left;
    dl_u8 right;
    dl_u8 pixel3;
    dl_u8 pixel0;
    dl_u16* destRow;


    // Lookup for 2-bit pairs ? color
    // bits: 00=BLACK, 01=BLUE, 10=ORANGE, 11=WHITE
    pairToColor[0] = BLACK;
    pairToColor[1] = BLUE;
    pairToColor[2] = ORANGE;
    pairToColor[3] = WHITE;

    for (y = 0; y < height; ++y)
    {
        yOffset = y * destinationWidth;

        // Decode + CRT effect in one pass
        // Process per byte of input (8 bits = 4 pairs)
        for (byteX = 0; byteX < bytesPerRow; ++byteX)
        {
            bits = originalImage[y * bytesPerRow + byteX];

            // Extract four 2-bit pairs, MSB first
            // Pair 0: bits 7,6
            // Pair 1: bits 5,4
            // Pair 2: bits 3,2
            // Pair 3: bits 1,0

            pairs[0] = (bits >> 6) & 0x3;
            pairs[1] = (bits >> 4) & 0x3;
            pairs[2] = (bits >> 2) & 0x3;
            pairs[3] = (bits >> 0) & 0x3;

            // Decode colors (each pair repeated twice)
            baseX = byteX * 8;
            for (i = 0; i < 4; ++i)
            {
                dl_u8 color = pairToColor[pairs[i]];
                rowBuffer[baseX + i * 2]     = color;
                rowBuffer[baseX + i * 2 + 1] = color;
            }
        }

        // Apply CRT artifact effect in one pass
        // Note: safe to process full width - 2 because we check bounds in condition
        for (x = 0; x < width; x += 2)
        {
            left  = rowBuffer[x];
            right = rowBuffer[x + 1];

            if (right == BLUE && x < width - 2)
            {
                pixel3 = rowBuffer[x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    left  = BLACK;
                    right = WHITE;
                }
            }
            else if (left == ORANGE && x >= 2)
            {
                pixel0 = rowBuffer[x - 1];
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
        destRow = (dl_u16*)(destinationImage + yOffset);
        for (x = 0; x < width; x += 2)
        {
            destRow[x / 2] = (rowBuffer[x] << 8) | rowBuffer[x + 1];
        }
    }
}
