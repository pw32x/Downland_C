// DLExporter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstddef> // for std::byte
#include <stdexcept>
#include <string>
#include <format>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

extern "C"
{
#include "base_types.h"
#include "base_defines.h"
#include "resource_types.h"
#include "resource_loader_buffer.h"
#include "draw_utils.h"

void* dl_alloc(dl_u32 size)
{
	return malloc(size);
}

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{
    memset(source, value, count);
}

void dl_memcpy(void* destination, const void* source, dl_u16 count)
{
    memcpy(destination, source, count);
}
   
}


std::vector<dl_u8> load_binary_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        throw std::runtime_error("Failed to open file: " + path);

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<dl_u8> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        throw std::runtime_error("Failed to read file: " + path);

    return buffer;
}


enum CrtColor
{
    CrtColor_Blue,
    CrtColor_Orange
};


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


dl_u16 findDuplicateTile(const dl_u8 tile[32], 
                         const dl_u16* vramTileAddr, 
                         dl_u16 maxTiles)
{
    const dl_u16* tile16 = (const dl_u16*)tile;

    dl_u16 loop = 0;

    for (loop = 0; loop < maxTiles; loop++)
    {
        dl_u16 differences = 0;

        for (dl_u8 counter = 0; counter < 16; counter++)
        {
            differences += tile16[counter] ^ vramTileAddr[counter];
            if (differences)
                break;
        }

        if (!differences)
        {
            return loop;
        }

        vramTileAddr += 16;
    }

    return 0xffff;
}

dl_u16 convertBackgroundToVRAM16(const dl_u8* originalImage,
                                 dl_u16* vramTileAddr,
                                 dl_u16* vramTileMapAddr,
                                 dl_u16 startTileOffset,
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

    dl_u8 tile[32];

    dl_u16 tilesCreated = 0;

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

            // if the tile isn't empty, then check for duplicates
            // if there is then use that
            if (sumColor)
            {
                dl_u16 duplicateTileIndex = findDuplicateTile(tile, 
                                                              vramTileAddr, 
                                                              startTileOffset + tilesCreated);

                if (duplicateTileIndex == 0xffff)
                {
                    tileIndex = tilesCreated + startTileOffset;
                    //CpuFastSet(tile, vramTileAddr + (tileIndex * 16), COPY32 | 8);
                    throw std::exception("not implemented yet");
                    tilesCreated++;
                }
                else
                {
                    tileIndex = duplicateTileIndex;
                }
            }

            vramTileMapAddr[tileX + (tileY * 32)] = tileIndex;            
        }
    }

    return startTileOffset + tilesCreated;
}

bool save8bppToPng(const uint8_t* buffer8bpp,
                   int width, int height,
                   const uint32_t* paletteRGBA, // 16 entries, 0xAARRGGBB
                   const std::string& filename)
{
    std::vector<uint8_t> rgba(width * height * 4);

    for (int i = 0; i < width * height; ++i)
    {
        uint32_t color = paletteRGBA[buffer8bpp[i]];
        rgba[i * 4 + 0] = (color >> 16) & 0xFF; // R
        rgba[i * 4 + 1] = (color >> 8)  & 0xFF; // G
        rgba[i * 4 + 2] = (color >> 0)  & 0xFF; // B
        rgba[i * 4 + 3] = (color >> 24) & 0xFF; // A
    }

    return stbi_write_png(filename.c_str(), width, height, 4, rgba.data(), width * 4) != 0;
}

// downland colors
uint32_t palette[16] =
{
    0xFF000000, // black
    0xFF0000FF, // blue
    0xFFFFA500, // orange
    0xFFFFFFFF, // white
    0xFF000000, 
    0xFF000000, 
    0xFF000000, 
    0xFF000000,
    0xFF000000, 
    0xFF000000, 
    0xFF000000, 
    0xFF000000,
    0xFF000000, 
    0xFF000000, 
    0xFF000000, 
    0xFF000000
};

int main()
{
    Resources resources;
    std::cout << "Hello World!\n";

    std::filesystem::path cwd = std::filesystem::current_path();
    std::cout << "Current working directory: " << cwd << "\n";

    auto downland_rom = load_binary_file("..\\..\\res\\downland.rom");

    ResourceLoaderBuffer_Init(downland_rom.data(), DOWNLAND_ROM_SIZE, &resources);

    dl_u8 framebuffer[FRAMEBUFFER_PITCH * FRAMEBUFFER_HEIGHT];
    dl_u8 framebuffer8bpp[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
    {
        drawBackground(&resources.roomResources[loop].backgroundDrawData, 
				       &resources,
				       framebuffer);

        convert1bppImageTo8bppCrtEffectImage(framebuffer,
                                             framebuffer8bpp,
                                             FRAMEBUFFER_WIDTH,
                                             FRAMEBUFFER_HEIGHT,
                                             CrtColor_Blue);

        save8bppToPng(framebuffer8bpp, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, palette,  std::format("room{}.png", loop));
    }

}
