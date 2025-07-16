#include "image_utils.h"

void convert1bppImageTo2bppCrtEffectImage(const dl_u8* originalImage,
                                          dl_u8* destinationImage,
                                          dl_u16 width,
                                          dl_u16 height,
                                          dl_u16 destinationBufferWidth,
                                          enum CrtColor crtColor)
{
    dl_u16 bytesPerRow = width / 8;
    dl_u16 outputBytesPerRow = destinationBufferWidth / 4;

    dl_u8 BLACK  = 0x0;
    dl_u8 BLUE   = (crtColor == CrtColor_Blue) ? 0x1 : 0x2;
    dl_u8 ORANGE = (crtColor == CrtColor_Blue) ? 0x2 : 0x1;
    dl_u8 WHITE  = 0x3;

    dl_u32 y;
    dl_u16 x;
    dl_u32 byteIndex, bitOffset;
    dl_u8 bit1, bit2;
    dl_u8 color;
    dl_u16 dstByteIndex;
    dl_u8 shift, mask, value;
    dl_u8 pixel0, pixel1;
    dl_u8 nextColor, prevColor;

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; x += 2)
        {
            // Read two bits from 1bpp source
            byteIndex = (y * bytesPerRow) + (x / 8);
            bitOffset = 7 - (x % 8);
            bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
            bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

            // Convert bits to color
            color = BLACK;
            if (bit1 == 0 && bit2 == 1)
                color = BLUE;
            else if (bit1 == 1 && bit2 == 0)
                color = ORANGE;
            else if (bit1 == 1 && bit2 == 1)
                color = WHITE;

            pixel0 = color;
            pixel1 = color;

            // CRT effect simulation (lookahead/lookbehind)
            if (color == BLUE && x + 2 < width)
            {
                byteIndex = (y * bytesPerRow) + ((x + 2) / 8);
                bitOffset = 7 - ((x + 2) % 8);
                bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
                bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

                nextColor = BLACK;
                if (bit1 == 0 && bit2 == 1)
                    nextColor = BLUE;
                else if (bit1 == 1 && bit2 == 0)
                    nextColor = ORANGE;
                else if (bit1 == 1 && bit2 == 1)
                    nextColor = WHITE;

                if (nextColor == ORANGE || nextColor == WHITE)
                {
                    pixel0 = BLACK;
                    pixel1 = WHITE;
                }
            }
            else if (color == ORANGE && x >= 2)
            {
                byteIndex = (y * bytesPerRow) + ((x - 2) / 8);
                bitOffset = 7 - ((x - 2) % 8);
                bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
                bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

                prevColor = BLACK;
                if (bit1 == 0 && bit2 == 1)
                    prevColor = BLUE;
                else if (bit1 == 1 && bit2 == 0)
                    prevColor = ORANGE;
                else if (bit1 == 1 && bit2 == 1)
                    prevColor = WHITE;

                if (prevColor == BLUE || prevColor == WHITE)
                {
                    pixel0 = WHITE;
                    pixel1 = BLACK;
                }
            }

            // Store pixel0
            dstByteIndex = (y * outputBytesPerRow) + (x / 4);
            shift = 6 - 2 * (x % 4);
            mask = 0x3 << shift;
            value = (pixel0 & 0x3) << shift;
            destinationImage[dstByteIndex] = (destinationImage[dstByteIndex] & (~mask)) | value;

            // Store pixel1 (could go into next byte)
            shift -= 2;
            if (shift < 8)
            {
                if (shift < 0)
                {
                    dstByteIndex++;
                    shift += 8;
                }
                mask = 0x3 << shift;
                value = (pixel1 & 0x3) << shift;
                destinationImage[dstByteIndex] = (destinationImage[dstByteIndex] & (~mask)) | value;
            }
        }
    }
}
void convert1bppImageTo2bppBlueImage(const dl_u8* originalImage,
                                     dl_u8* destinationImage,
                                     dl_u16 width,
                                     dl_u16 height,
                                     dl_u16 destinationBufferWidth)
{
    dl_u16 bytesPerRow = width / 8;
    dl_u16 outputBytesPerRow = destinationBufferWidth / 4;

    dl_u8 BLACK  = 0x0;
    dl_u8 BLUE   = 0x1;

    dl_u32 y;
    dl_u16 x;
    dl_u32 byteIndex, bitOffset;
    dl_u8 bit1, bit2;
    dl_u8 color;
    dl_u16 dstByteIndex;
    dl_u8 shift, mask, value;
    dl_u8 pixel0, pixel1;
    dl_u8 nextColor, prevColor;

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; x += 2)
        {
            // Read two bits from 1bpp source
            byteIndex = (y * bytesPerRow) + (x / 8);
            bitOffset = 7 - (x % 8);
            bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
            bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

            // Convert bits to color
            color = BLACK;
            if (bit1 == 1 || bit2 == 1)
                color = BLUE;

            pixel0 = color;
            pixel1 = color;

            // Store pixel0
            dstByteIndex = (y * outputBytesPerRow) + (x / 4);
            shift = 6 - 2 * (x % 4);
            mask = 0x3 << shift;
            value = (pixel0 & 0x3) << shift;
            destinationImage[dstByteIndex] = (destinationImage[dstByteIndex] & (~mask)) | value;

            // Store pixel1 (could go into next byte)
            shift -= 2;
            if (shift < 8)
            {
                if (shift < 0)
                {
                    dstByteIndex++;
                    shift += 8;
                }
                mask = 0x3 << shift;
                value = (pixel1 & 0x3) << shift;
                destinationImage[dstByteIndex] = (destinationImage[dstByteIndex] & (~mask)) | value;
            }
        }
    }
}