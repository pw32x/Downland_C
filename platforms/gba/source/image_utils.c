#include "image_utils.h"

#include <gba_video.h>
#include <gba_systemcalls.h>

#include <string.h>

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

    for (int y = 0; y < height; ++y) 
    {
        // every pair of bits generates a color for the two corresponding
        // pixels of the destination texture, so:
        // source bits:        00 01 10 11
        // final pixel colors: black, black, blue, blue, orange, orange, white, white.
        dl_u32 yOffset = y * width;

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

void convert1bppImageToVRAM(const dl_u8* originalImage,
                            dl_u16* vram,
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

    for (int y = 0; y < height; ++y) 
    {
        // every pair of bits generates a color for the two corresponding
        // pixels of the destination texture, so:
        // source bits:        00 01 10 11
        // final pixel colors: black, black, blue, blue, orange, orange, white, white.
        dl_u16 tileWidth = width / 8;

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
            dl_u16 tileX = x / 8;
            dl_u16 tileY = y / 8;
            dl_u8 rowX = x % 8;
            dl_u8 rowY = y % 8;

            dl_u16 tileIndex = tileX + (tileY * tileWidth);
            dl_u16 byteLocation = ((tileIndex * 64) + rowX + (rowY * 8)) >> 1;

            vram[byteLocation] = color | (color << 8);
        }
    }
}

void convertBackgroundToVRAM256(const dl_u8* originalImage,
                                dl_u16* vramTileAddr,
                                dl_u16* vramTileMapAddr,
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

    dl_u16 tileWidth = width / 8;
    dl_u16 tileHeight = height / 8;

    dl_u8 tile[64]; // 256 colors
    memset(tile, 0, sizeof(tile));
    CpuFastSet(tile, vramTileAddr, COPY32 | 16);

    dl_u16 tileCounter = 1;

    for (dl_u16 tileY = 0; tileY < tileHeight; tileY++) 
    {
        for (dl_u16 tileX = 0; tileX < tileWidth; tileX++)
        {
            dl_u16 startX = tileX;
            dl_u16 startY = tileY * 8;

            dl_u16 sumColor = 0;

            for (dl_u8 y = 0; y < 8; y++) 
            {
                // every pair of bits generates a color for the two corresponding
                // pixels of the destination texture, so:
                // source bits:        00 01 10 11
                // final pixel colors: black, black, blue, blue, orange, orange, white, white.
                for (dl_u8 x = 0; x < 8; x += 2) 
                {
                    int byteIndex = ((startY + y) * bytesPerRow) + startX;
                    int bitOffset = 7 - (x % 8);
                    
                    // Read two adjacent bits
                    dl_u8 bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
                    dl_u8 bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

                    // Determine color
                    dl_u8 color = BLACK;
                    if (bit1 == 0 && bit2 == 1) color = BLUE;
                    else if (bit1 == 1 && bit2 == 0) color = ORANGE;
                    else if (bit1 == 1 && bit2 == 1) color = WHITE;

                    sumColor += color;

                    // Set color
                    dl_u16 byteLocation = x + (y * 8);
                    tile[byteLocation] = color;
                    tile[byteLocation + 1] = color;
                }
            }

            dl_u16 tileIndex = 0;

            if (sumColor)
            {
                tileIndex = tileCounter;
                CpuFastSet(tile, vramTileAddr + (tileCounter * 32), COPY32 | 16);
                tileCounter++;
            }

            vramTileMapAddr[tileX + (tileY * 32)] = tileIndex;            
        }
    }
}


void convertBackgroundToVRAM16(const dl_u8* originalImage,
                               dl_u16* vramTileAddr,
                               dl_u16* vramTileMapAddr,
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

    dl_u16 tileWidth = width / 8;
    dl_u16 tileHeight = height / 8;

    dl_u8 tile[32]; // 16 colors
    memset(tile, 0, sizeof(tile));
    CpuFastSet(tile, vramTileAddr, COPY32 | 8);

    dl_u16 tileCounter = 1;

    for (dl_u16 tileY = 0; tileY < tileHeight; tileY++) 
    {
        for (dl_u16 tileX = 0; tileX < tileWidth; tileX++)
        {
            dl_u16 startX = tileX;
            dl_u16 startY = tileY * 8;

            dl_u16 sumColor = 0;

            for (dl_u8 y = 0; y < 8; y++) 
            {
                // every pair of bits generates a color for the two corresponding
                // pixels of the destination texture, so:
                // source bits:        00 01 10 11
                // final pixel colors: black, black, blue, blue, orange, orange, white, white.
                for (dl_u8 x = 0; x < 8; x += 2) 
                {
                    int byteIndex = ((startY + y) * bytesPerRow) + startX;
                    int bitOffset = 7 - (x % 8);
                    
                    // Read two adjacent bits
                    dl_u8 bit1 = (originalImage[byteIndex] >> bitOffset) & 1;
                    dl_u8 bit2 = (originalImage[byteIndex] >> (bitOffset - 1)) & 1;

                    // Determine color
                    dl_u8 color = BLACK;
                    if (bit1 == 0 && bit2 == 1) color = BLUE;
                    else if (bit1 == 1 && bit2 == 0) color = ORANGE;
                    else if (bit1 == 1 && bit2 == 1) color = WHITE;

                    sumColor += color;

                    // Set color
                    dl_u16 byteLocation = (x + (y * 8)) >> 1;
                    tile[byteLocation] = color | (color << 4);
                }
            }

            dl_u16 tileIndex = 0;

            if (sumColor)
            {
                tileIndex = tileCounter;
                CpuFastSet(tile, vramTileAddr + (tileCounter * 16), COPY32 | 8);
                tileCounter++;
            }

            vramTileMapAddr[tileX + (tileY * 32)] = tileIndex;            
        }
    }
}

dl_u16 convertToTiles(const dl_u8* sprite, 
					  dl_u16 width,
					  dl_u16 height,
                      void* vramLocation,
					  dl_u16 offsetInBytes)
{
	dl_u8 tileWidth = (width + 7) / 8;
	dl_u8 tileHeight = (height + 7) / 8;

	// vram only accepts 16bit writes
	dl_u16* vramRunner = (dl_u16*)(vramLocation + offsetInBytes);

    dl_u16 tileCount = 0;

	for (int tiley = 0; tiley < tileHeight; tiley++)
	{
		int starty = tiley * 8;

		for (int tilex = 0; tilex < tileWidth; tilex++)
		{
			int startx = tilex * 8;

            tileCount++;

			for (int loopy = 0; loopy < 8; loopy++)
			{
				for (int loopx = 0; loopx < 8; loopx += 2)
				{
					// write two pixels at a time
					int pixelIndex = (startx + loopx) + ((starty + loopy) * width);
					*vramRunner = sprite[pixelIndex] | (sprite[pixelIndex + 1] << 8);
					vramRunner++;
				}
			}
		}
	}

    return tileCount;
}
