#include "image_utils.h"

void convert1bppImageTo8bppCrtEffect(const dl_u8* originalImage,
                                           dl_u8* destinationImage,
                                           dl_u16 width,
                                           dl_u16 height,
                                           dl_u16 destinationWidth,
                                           dl_u8 blackIndex)
{
    #define RAW_BLACK  0
    #define RAW_BLUE   1
    #define RAW_ORANGE 2
    #define RAW_WHITE  3

    static dl_u8 pairTable[256][4];
    static int pairTableInitialized = 0;

    dl_u8 pairColors[160];
    const dl_u8 bytesPerRow = width / 8;
    const dl_u16 numPairs = width / 2;

    int y;
    int yOffset;
    int byteX;
    int i;
    const dl_u8* p;
    register dl_u8 cur;
    register dl_u8 outLeft;
    register dl_u8 outRight;
    register dl_u8 prevRight;
    register dl_u8 l;
    register dl_u8 r;
    dl_u16* destRow;

    if (!pairTableInitialized)
    {
        int b;
        for (b = 0; b < 256; ++b)
        {
            pairTable[b][0] = (dl_u8)((b >> 6) & 0x3);
            pairTable[b][1] = (dl_u8)((b >> 4) & 0x3);
            pairTable[b][2] = (dl_u8)((b >> 2) & 0x3);
            pairTable[b][3] = (dl_u8)((b >> 0) & 0x3);
        }
        pairTableInitialized = 1;
    }

    for (y = 0; y < height; ++y)
    {
        yOffset = y * destinationWidth;

        // Decode: one table lookup per source byte gives all 4 raw pair values
        for (byteX = 0; byteX < bytesPerRow; ++byteX)
        {
            p = pairTable[originalImage[y * bytesPerRow + byteX]];
            i = byteX * 4;
            pairColors[i]     = p[0];
            pairColors[i + 1] = p[1];
            pairColors[i + 2] = p[2];
            pairColors[i + 3] = p[3];
        }

        // Fused CRT-artifact effect + output write, single pass over pairs.
        // prevRight tracks the already-transformed right value of pair i-1,
        // matching the original's in-place left-to-right chaining, while the
        // lookahead to pair i+1 uses its untouched raw value (not yet visited).
        destRow = (dl_u16*)(destinationImage + yOffset);
        prevRight = RAW_BLACK; // never read at i==0 (guarded below); set to silence uninitialized-use warnings

        for (i = 0; i < numPairs; ++i)
        {
            cur = pairColors[i];
            outLeft = cur;
            outRight = cur;

            if (cur == RAW_BLUE && i + 1 < numPairs &&
                (pairColors[i + 1] == RAW_ORANGE || pairColors[i + 1] == RAW_WHITE))
            {
                outLeft  = RAW_BLACK;
                outRight = RAW_WHITE;
            }
            else if (cur == RAW_ORANGE && i > 0 &&
                     (prevRight == RAW_BLUE || prevRight == RAW_WHITE))
            {
                outLeft  = RAW_WHITE;
                outRight = RAW_BLACK;
            }

            prevRight = outRight;

            l = outLeft  ? outLeft  : blackIndex;
            r = outRight ? outRight : blackIndex;

            destRow[i] = (dl_u16)((l << 8) | r);
        }
    }
}
