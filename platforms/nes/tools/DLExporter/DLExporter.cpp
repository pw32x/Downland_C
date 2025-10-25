#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstddef> // for std::byte
#include <stdexcept>
#include <string>
#include <format>
#include <array>
#include <string_view>
#include <bitset>


#ifdef _WIN64
std::string g_destinationPath = "generated\\";
#else
std::string g_destinationPath = "generated/";
#endif

extern "C"
{
#include "base_types.h"
#include "base_defines.h"
#include "resource_types.h"
#include "resource_loader_buffer.h"
#include "draw_utils.h"
#include "drops_manager.h"
#include "rooms/chambers.h"

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

const char* bankFolderNames[] = 
{
    "bank2",
    "bank3",
    "bank4",
    "bank5",
    "bank6",
    "bank7",
};

int bankFolderNameCount = sizeof(bankFolderNames) / sizeof(bankFolderNames[0]);

const dl_u8 roomToBankFolderNameIndex[] = 
{
    0, // chambers 0 to 9
    0,
    1,
    1,
    2,
    2,
    3,
    3,
    4,
    4,
    5,  // title screen
    11, // transition
    0,  // wipe transition
    5   // get ready screen
};
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

#define TILE_WIDTH 8
#define TILE_PITCH 2
#define TILE_HEIGHT 8
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)

using Tile = std::array<dl_u8, TILE_SIZE>;
using TileSet = std::vector<Tile>;

#define TILE_MAP_WIDTH (FRAMEBUFFER_WIDTH / 8)
#define TILE_MAP_HEIGHT (FRAMEBUFFER_HEIGHT / 8)
#define TILE_MAP_SIZE (TILE_MAP_WIDTH * TILE_MAP_HEIGHT)

using TileMap = std::vector<dl_u16>;

void extractTile(const dl_u8* background, dl_u8 startX, dl_u8 startY, Tile& tile)
{
    for (dl_u16 loopy = 0; loopy < TILE_HEIGHT; loopy++)
    {
        for (dl_u16 loopx = 0; loopx < TILE_WIDTH; loopx++)
        {
            tile[loopx + (loopy * TILE_WIDTH)] = background[(loopx + startX) + ((loopy + startY) * FRAMEBUFFER_WIDTH)];
        }
    }
}

dl_u16 appendTileToTileSet(const Tile& tile, TileSet& tileSet)
{
    // if the tile already exists, just return the existing index
    dl_u16 tileSetIndex = 0;
    bool found = false;
    for (auto& tileSetTile : tileSet)
    {
        if (tileSetTile == tile)
        {
            found = true;
            break;
        }

        tileSetIndex++;
    }

    if (!found)
    {
        tileSetIndex = static_cast<dl_u16>(tileSet.size());
        tileSet.push_back(tile);
    }

    return tileSetIndex;
}

void buildTileMap(const dl_u8* background, TileMap& tileMap, TileSet& tileSet)
{
    for (int tileMapY = 0; tileMapY < TILE_MAP_HEIGHT; tileMapY++)
    {
        for (int tileMapX = 0; tileMapX < TILE_MAP_WIDTH; tileMapX++)
        {
            dl_u8 startX = tileMapX * TILE_WIDTH;
            dl_u8 startY = tileMapY * TILE_HEIGHT;

            Tile tile;
            extractTile(background, startX, startY, tile);

            dl_u16 tileIndex = appendTileToTileSet(tile, tileSet);

            tileMap.push_back(tileIndex);
        }
    }
}

dl_u8* writePlanarTile(const dl_u8* source, int sourceWidth, int tileHeight, dl_u8* chrBufferRunner)
{
    const dl_u8* tileRunner = source;

    // low bits first
    for (int y = 0; y < tileHeight; y++)
    {
        dl_u8 lowBits;

        lowBits = ((tileRunner[0] & 0b1) << 7) |
                  ((tileRunner[1] & 0b1) << 6) |
                  ((tileRunner[2] & 0b1) << 5) |
                  ((tileRunner[3] & 0b1) << 4) |
                  ((tileRunner[4] & 0b1) << 3) |
                  ((tileRunner[5] & 0b1) << 2) |
                  ((tileRunner[6] & 0b1) << 1) |
                   (tileRunner[7] & 0b1);

        tileRunner += sourceWidth;

        *chrBufferRunner = lowBits;
        chrBufferRunner++;
    }

    // high bits
    tileRunner = source;

    for (int y = 0; y < tileHeight; y++)
    {
        dl_u8 highBits;

        highBits = (((tileRunner[0] & 0b10) >> 1) << 7) |
                   (((tileRunner[1] & 0b10) >> 1) << 6) |
                   (((tileRunner[2] & 0b10) >> 1) << 5) |
                   (((tileRunner[3] & 0b10) >> 1) << 4) |
                   (((tileRunner[4] & 0b10) >> 1) << 3) |
                   (((tileRunner[5] & 0b10) >> 1) << 2) |
                   (((tileRunner[6] & 0b10) >> 1) << 1) |
                    ((tileRunner[7] & 0b10) >> 1);

        tileRunner += sourceWidth;

        *chrBufferRunner = highBits;
        chrBufferRunner++;
    }

    return chrBufferRunner;
}

dl_u8* saveTileSetToChr(const TileSet& tileSet, dl_u8* chrBufferRunner)
{
    for (const auto& tile : tileSet)
    {
        chrBufferRunner = writePlanarTile(tile.data(), TILE_WIDTH, TILE_HEIGHT, chrBufferRunner);
    }

    return chrBufferRunner;
}


dl_u8* saveCharacterFontToChr(const dl_u8* characterFont, dl_u8* chrBufferRunner)
{
    for (int characterLoop = 0; characterLoop < CHARACTER_FONT_COUNT; characterLoop++)
    {
        for (int loopy = 0; loopy < CHARACTER_FONT_HEIGHT; loopy++)
        {
            // low bits
            *chrBufferRunner = characterFont[loopy];
            chrBufferRunner++;
        }

        characterFont += CHARACTER_FONT_HEIGHT; // move to next character

        // last row of low bits
        *chrBufferRunner = 0;
        chrBufferRunner++;

        // high bits
        for (int loopy = 0; loopy < TILE_HEIGHT; loopy++)
        {
            *chrBufferRunner = 0;
            chrBufferRunner++;        
        }
    }

    return chrBufferRunner;
}


dl_u8* writeEmptyTileToChr(dl_u8* chrBufferRunner)
{
    // low bits
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;

    // high bits
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;
    *chrBufferRunner = 0; chrBufferRunner++;

    return chrBufferRunner;
}

dl_u8* saveSpriteToChr(const dl_u8* sprite, 
                         dl_u8 width, 
                         dl_u8 height, 
                         dl_u8 numFrames,
                         dl_u8* chrBufferRunner,
                         bool writeEmptyBottomTile = false)
{
    dl_u8 spriteFrameSizeInBytes = (width / 8) * height;

    dl_u8 destinationWidth = ((width + 7) / 8) * 8;
    dl_u8 destinationHeight = ((height + 7) / 8) * 8;
    dl_u16 destinationFrameSize = destinationWidth * destinationHeight;

    dl_u16 bufferSize = destinationWidth * destinationHeight * numFrames;
    dl_u8* sprite8bpp = new dl_u8[bufferSize];
    memset(sprite8bpp, 0, bufferSize);
    dl_u8* sprite8bppRunner = sprite8bpp;


    for (int frameLoop = 0; frameLoop < numFrames; frameLoop++)
    {
        convert1bppImageTo8bppCrtEffectImage(sprite,
                                             sprite8bppRunner,
                                             width,
                                             height,
                                             CrtColor::CrtColor_Blue);


        dl_u8 tileWidth = destinationWidth / TILE_WIDTH;
        dl_u8 tileHeight = destinationHeight / TILE_HEIGHT;
        
        for (int tilex = 0; tilex < tileWidth; tilex++)
        {
            for (int tiley = 0; tiley < tileHeight; tiley++)    
            {
                const dl_u8* tileStart = sprite8bppRunner + ((tilex * TILE_WIDTH) + ((tiley * TILE_HEIGHT) * destinationWidth));
                chrBufferRunner = writePlanarTile(tileStart, destinationWidth, TILE_HEIGHT, chrBufferRunner);

                if (writeEmptyBottomTile)
                    chrBufferRunner = writeEmptyTileToChr(chrBufferRunner);
            }
        }

        // move to next frame
        sprite += spriteFrameSizeInBytes;
        sprite8bppRunner += destinationFrameSize; 
    }

    /*
    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * (numFrames * 2),
                  g_destinationPath + "sprite.png");
    */

    delete [] sprite8bpp;

    return chrBufferRunner;
}


dl_u8* saveCursorToChr(dl_u8* chrBufferRunner)
{
    // top tile
    // low bits
    *chrBufferRunner = 0xff; chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;

    // high bits
    *chrBufferRunner = 0xff; chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;
    *chrBufferRunner = 0;    chrBufferRunner++;

    // bottom tile
    return writeEmptyTileToChr(chrBufferRunner);
}

#define NUM_REGEN_FRAMES 4


dl_u8* saveRegenSpriteToChr(const dl_u8* playerSprite, dl_u8** chrBufferRunnerPtr)
{
    const int numFrames = NUM_REGEN_FRAMES;
    const int width = 16;
    const int height = 16;

    // drawSprite_16PixelsWide_static_IntoSpriteBuffer

    dl_u8 spriteFrameSizeInBytes = (width / 8) * height;

    dl_u8 destinationWidth = ((width + 7) / 8) * 8;
    dl_u8 destinationHeight = ((height + 7) / 8) * 8;
    dl_u16 destinationFrameSize = destinationWidth * destinationHeight;

    dl_u16 bufferSize = destinationWidth * destinationHeight * (numFrames * 2);
    dl_u8* sprite8bpp = new dl_u8[bufferSize];
    memset(sprite8bpp, 0, bufferSize);
    dl_u8* sprite8bppRunner = sprite8bpp;

    dl_u16 regenBufferSize = (width / 8) * height;
    dl_u8* regenBuffer = new dl_u8[regenBufferSize];

    for (int frameLoop = 0; frameLoop < numFrames; frameLoop++)
    {
        memset(regenBuffer, 0, regenBufferSize);

        drawSprite_16PixelsWide_static_IntoSpriteBuffer(playerSprite,
                                                        height,
                                                        regenBuffer);

        convert1bppImageTo8bppCrtEffectImage(regenBuffer,
                                             sprite8bppRunner,
                                             width,
                                             height,
                                             CrtColor::CrtColor_Blue);

        // write to chr
        dl_u8 tileWidth = destinationWidth / TILE_WIDTH;
        dl_u8 tileHeight = destinationHeight / TILE_HEIGHT;
        
        for (int tilex = 0; tilex < tileWidth; tilex++)
        {
            for (int tiley = 0; tiley < tileHeight; tiley++)    
            {
                const dl_u8* tileStart = sprite8bppRunner + ((tilex * TILE_WIDTH) + ((tiley * TILE_HEIGHT) * destinationWidth));
                *chrBufferRunnerPtr = writePlanarTile(tileStart, destinationWidth, TILE_HEIGHT, *chrBufferRunnerPtr);
            }
        }


        // move to next frame
        sprite8bppRunner += destinationFrameSize; 
    }

    // left side standing
    playerSprite += ((width / 8) * height) * 6; // PLAYER_SPRITE_LEFT_STAND

    for (int frameLoop = 0; frameLoop < numFrames; frameLoop++)
    {
        memset(regenBuffer, 0, regenBufferSize);

        drawSprite_16PixelsWide_static_IntoSpriteBuffer(playerSprite,
                                                        height,
                                                        regenBuffer);

        convert1bppImageTo8bppCrtEffectImage(regenBuffer,
                                             sprite8bppRunner,
                                             width,
                                             height,
                                             CrtColor::CrtColor_Blue);

        // write to chr
        dl_u8 tileWidth = destinationWidth / TILE_WIDTH;
        dl_u8 tileHeight = destinationHeight / TILE_HEIGHT;
        
        for (int tilex = 0; tilex < tileWidth; tilex++)
        {
            for (int tiley = 0; tiley < tileHeight; tiley++)    
            {
                const dl_u8* tileStart = sprite8bppRunner + ((tilex * TILE_WIDTH) + ((tiley * TILE_HEIGHT) * destinationWidth));
                *chrBufferRunnerPtr = writePlanarTile(tileStart, destinationWidth, TILE_HEIGHT, *chrBufferRunnerPtr);
            }
        }

        // move to next frame
        sprite8bppRunner += destinationFrameSize; 
    }

    /*
    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * (numFrames * 2),
                  g_destinationPath + "regenTileset.png");
    */

    delete [] regenBuffer;

    return sprite8bpp;
}


dl_u8* saveRegenLivesSpriteToChr(dl_u8* regenSprite, dl_u8* chrBufferRunner)
{
    const int numFrames = NUM_REGEN_FRAMES;
    const int regenWidth = 16;
    const int regenHeight = 16;
    const int iconImageHeight = 8;
    const int iconHeight = 7;

    const int regenSpriteFrameSize = regenWidth * regenHeight;
    const int regenIconFrameSize = regenWidth * iconImageHeight;

    dl_u8* regenIconBuffer = new dl_u8[regenIconFrameSize * numFrames];
    memset(regenIconBuffer, 0, regenIconFrameSize * numFrames);
    dl_u8* regenIconBufferRunner = regenIconBuffer;

    for (int frameLoop = 0; frameLoop < numFrames; frameLoop++)
    {
        memcpy(regenIconBufferRunner, regenSprite, regenIconFrameSize);

        chrBufferRunner = writePlanarTile(regenSprite, regenWidth, TILE_HEIGHT, chrBufferRunner);
        chrBufferRunner = writeEmptyTileToChr(chrBufferRunner);
        chrBufferRunner = writePlanarTile(regenSprite + 8, regenWidth, TILE_HEIGHT, chrBufferRunner);
        chrBufferRunner = writeEmptyTileToChr(chrBufferRunner);

        regenSprite += regenSpriteFrameSize;
        regenIconBufferRunner += regenIconFrameSize;
    }

    /*
    save_png_8bpp(regenIconBuffer, 
                  regenWidth,
                  iconImageHeight * numFrames,
                  g_destinationPath + "playerLivesRegenTileset.png");
    */

    delete [] regenIconBuffer;

    return chrBufferRunner;
}

dl_u8* saveSplatSpriteToChr(const dl_u8* splatSprite, dl_u8* chrBufferRunner)
{
    const int numFrames = 2; // original + modified
    const int width = 24;
    const int height = 9;

    dl_u8 destinationWidth = ((width + 7) / 8) * 8;
    dl_u8 destinationHeight = ((height + 7) / 8) * 8;
    dl_u16 destinationFrameSize = destinationWidth * destinationHeight;

    dl_u16 bufferSize = destinationWidth * destinationHeight * numFrames;
    dl_u8* sprite8bpp = new dl_u8[bufferSize];
    memset(sprite8bpp, 0, bufferSize);

    convert1bppImageTo8bppCrtEffectImage(splatSprite,
                                         sprite8bpp,
                                         width,
                                         height,
                                         CrtColor::CrtColor_Blue);

    // write to chr
    dl_u8 tileWidth = destinationWidth / TILE_WIDTH;
    dl_u8 tileHeight = destinationHeight / TILE_HEIGHT;
        
    for (int tilex = 0; tilex < tileWidth; tilex++)
    {
        for (int tiley = 0; tiley < tileHeight; tiley++)    
        {
            const dl_u8* tileStart = sprite8bpp + ((tilex * TILE_WIDTH) + ((tiley * TILE_HEIGHT) * destinationWidth));
            chrBufferRunner = writePlanarTile(tileStart, destinationWidth, TILE_HEIGHT, chrBufferRunner);
        }
    }

    dl_u8* secondFrame = sprite8bpp + destinationFrameSize;

    // write it again but on the next frame
    convert1bppImageTo8bppCrtEffectImage(splatSprite,
                                         secondFrame,
                                         width,
                                         height,
                                         CrtColor::CrtColor_Blue);

    // clear the top five rows
    memset(secondFrame, 0, 24 * 5);

    for (int tilex = 0; tilex < tileWidth; tilex++)
    {
        for (int tiley = 0; tiley < tileHeight; tiley++)    
        {
            const dl_u8* tileStart = sprite8bpp + ((tilex * TILE_WIDTH) + ((tiley * TILE_HEIGHT) * destinationWidth));
            chrBufferRunner = writePlanarTile(tileStart, destinationWidth, TILE_HEIGHT, chrBufferRunner);
        }
    }

    /*
    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * numFrames,
                  g_destinationPath + "playerSplatTileset.png");
    */

    delete [] sprite8bpp;

    return chrBufferRunner;
}



dl_u8* saveSprite16ClippedToChr(const dl_u8* sprite, 
                                dl_u8 width, 
                                dl_u8 height, 
                                dl_u8 clipHeight,
                                dl_u8 numFrames,
                                dl_u8* chrBufferRunner)
{
    dl_u8 spriteFrameSizeInBytes = (width / 8) * height;

    dl_u8 destinationWidth = ((width + 7) / 8) * 8;
    dl_u8 destinationHeight = ((clipHeight + 7) / 8) * 8;
    dl_u16 destinationFrameSize = destinationWidth * destinationHeight;

    dl_u16 bufferSize = destinationWidth * destinationHeight * numFrames;
    dl_u8* sprite8bpp = new dl_u8[bufferSize];
    memset(sprite8bpp, 0, bufferSize);
    dl_u8* sprite8bppRunner = sprite8bpp;


    for (int frameLoop = 0; frameLoop < numFrames; frameLoop++)
    {
        convert1bppImageTo8bppCrtEffectImage(sprite,
                                             sprite8bppRunner,
                                             width,
                                             clipHeight,
                                             CrtColor::CrtColor_Blue);

        dl_u8 tileWidth = destinationWidth / TILE_WIDTH;
        dl_u8 tileHeight = destinationHeight / TILE_HEIGHT;
        
        for (int tilex = 0; tilex < tileWidth; tilex++)
        {
            for (int tiley = 0; tiley < tileHeight; tiley++)    
            {
                const dl_u8* tileStart = sprite8bppRunner + ((tilex * TILE_WIDTH) + ((tiley * TILE_HEIGHT) * destinationWidth));
                chrBufferRunner = writePlanarTile(tileStart, destinationWidth, TILE_HEIGHT, chrBufferRunner);
                chrBufferRunner = writeEmptyTileToChr(chrBufferRunner);
            }
        }

        // move to next frame
        sprite += spriteFrameSizeInBytes;
        sprite8bppRunner += destinationFrameSize; 
    }

    /*
    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * numFrames,
                  g_destinationPath + name + ".png");
    */

    delete [] sprite8bpp;

    return chrBufferRunner;
}

std::string roomNames[] = 
{
"chamber0",
"chamber1",
"chamber2",
"chamber3",
"chamber4",
"chamber5",
"chamber6",
"chamber7",
"chamber8",
"chamber9",
"titleScreen",
"none",
"none",
"getReadyScreen"
};

void saveTileMapSource(const std::vector<TileMap>& tileMaps)
{
    dl_u8 counter = 0;
    for (auto& tileMap : tileMaps)
    {
    std::ostringstream oss;

    oss << "#include \"base_types.h\"\n";
    oss << "\n";

        oss << "const dl_u8 " << roomNames[counter] << "_tileMap[32 * 24] = \n";
        oss << "{\n";

        for (int loopy = 0; loopy < TILE_MAP_HEIGHT; loopy++)
        {
            oss << "    ";

            for (int loopx = 0; loopx < TILE_MAP_WIDTH; loopx++)
            {
                dl_u16 tileIndex = tileMap[loopx + (loopy * TILE_MAP_WIDTH)];

                if (tileIndex > 255)
                {
                    throw std::runtime_error("tile index over 255");
                }

                oss << tileIndex << ", ";
            }

            oss << "\n";
        }

        oss << "};\n";
        oss << "\n";

        std::string bankFolder = g_destinationPath;
        bankFolder += bankFolderNames[roomToBankFolderNameIndex[counter]];
        bankFolder += "\\";

        std::ofstream outFile(bankFolder + roomNames[counter] + "_tileMap.c");
        outFile << oss.str();

        counter++;
    }

    /*
    oss << "\n";
    oss << "const dl_u8* roomTileMaps[" << NUM_ROOMS_PLUS_TITLESCREN << "] = \n";
    oss << "{\n";
    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
        oss << "    " << roomNames[loop] << "TileMap,\n";
    oss << "};\n";
    */



}

void saveCleanBackground(const dl_u8* cleanBackground, dl_u8 backgroundIndex)
{
    std::ostringstream oss;

    std::string cleanBackgroundName = roomNames[backgroundIndex];
    cleanBackgroundName += "_cleanBackground";

    oss << "#include \"base_types.h\"\n";
    oss << "\n";

    dl_u16 backgroundSize = FRAMEBUFFER_PITCH * FRAMEBUFFER_HEIGHT;

    const dl_u8* cleanBackgroundRunner = cleanBackground;

    oss << "const dl_u8 " << cleanBackgroundName << "[" << backgroundSize << "] = \n";
    oss << "{\n";

    for (int loopy = 0; loopy < FRAMEBUFFER_HEIGHT; loopy++)
    {
        oss << "    ";

        for (int loopx = 0; loopx < FRAMEBUFFER_PITCH; loopx++)
        {
            oss << (dl_u16)(*cleanBackgroundRunner) << ", ";
            cleanBackgroundRunner++;
        }

        oss << "\n";
    }

    oss << "};\n";
    oss << "\n";
    oss << "\n";

    oss << "// Map: \n";
    cleanBackgroundRunner = cleanBackground;
    for (int loopy = 0; loopy < FRAMEBUFFER_HEIGHT; loopy++)
    {
        oss << "// ";

        for (int loopx = 0; loopx < FRAMEBUFFER_PITCH; loopx++)
        {
            oss << std::bitset<8>(*cleanBackgroundRunner);
            cleanBackgroundRunner++;
        }

        oss << "\n";
    }
      
    std::string bankFolder = g_destinationPath;
    bankFolder += bankFolderNames[roomToBankFolderNameIndex[backgroundIndex]];
    bankFolder += "\\";

    std::ofstream outFile(bankFolder + cleanBackgroundName + ".c");
    outFile << oss.str();        
}

void saveString(const dl_u8* string, const char* name, std::ostringstream& oss)
{
    const dl_u8* stringRunner = string;
    while (*stringRunner != 0xff)
    {
        stringRunner++;
    }

    int length = (int)(stringRunner - string + 1);

    oss << "const dl_u8 res_string_" << name << "[" << std::dec << (dl_u16)length << "] = { ";
    
    while (1) 
    {
        oss  << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (dl_u16)*string;

        if (*string == 0xff)
        {
            break;
        }

        oss << ", ";

        string++;
    }

    oss << " };\n";
}

void saveStrings(const Resources& resources)
{
    std::ostringstream oss;

    oss << "#include \"base_types.h\"\n";
    oss << "\n";

    saveString(resources.text_downland, "downland", oss);
    saveString(resources.text_writtenBy, "writtenBy", oss);
    saveString(resources.text_michaelAichlmayer, "michaelAichlmayer", oss);
    saveString(resources.text_copyright1983, "copyright1983", oss);
    saveString(resources.text_spectralAssociates, "spectralAssociates", oss);
    saveString(resources.text_licensedTo, "licensedTo", oss);
    saveString(resources.text_tandyCorporation, "tandyCorporation", oss);
    saveString(resources.text_allRightsReserved, "allRightsReserved", oss);
    saveString(resources.text_onePlayer, "onePlayer", oss);
    saveString(resources.text_twoPlayer, "twoPlayer", oss);
    saveString(resources.text_highScore, "highScore", oss);
    saveString(resources.text_playerOne, "playerOne", oss);
    saveString(resources.text_playerTwo, "playerTwo", oss);
    saveString(resources.text_pl1, "pl1", oss);
    saveString(resources.text_pl2, "pl2", oss);
    saveString(resources.text_getReadyPlayerOne, "getReadyPlayerOne", oss);
    saveString(resources.text_getReadyPlayerTwo, "getReadyPlayerTwo", oss);
    saveString(resources.text_chamber, "chamber", oss);
            
    std::ofstream outFile(g_destinationPath + "strings.c");
    outFile << oss.str();
}


void saveDropSpawns(const Resources& resources)
{
    //resources.roomResources[0].dropSpawnPositions);

    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
    {
        std::ostringstream oss;

        std::string dropSpawnPositionsName = roomNames[loop];
        dropSpawnPositionsName += "_dropSpawnPositions";

        oss << "#include \"base_types.h\"\n";
        oss << "#include \"drops_types.h\"\n";
        oss << "\n";

        const DropSpawnPositions& dropSpawnPositions = resources.roomResources[loop].dropSpawnPositions;

        oss << "const DropSpawnArea " << dropSpawnPositionsName << "_array[" << (dl_u16)dropSpawnPositions.spawnAreasCount << "] = \n";  
        oss << "{\n";
        for (int innerLoop = 0; innerLoop < dropSpawnPositions.spawnAreasCount; innerLoop++)
        {
            oss << "    { ";
            oss << (dl_u16)dropSpawnPositions.dropSpawnAreas[innerLoop].dropSpawnPointsCount << ", ";
            oss << (dl_u16)dropSpawnPositions.dropSpawnAreas[innerLoop].x << ", ";
            oss << (dl_u16)dropSpawnPositions.dropSpawnAreas[innerLoop].y;
            oss << " }, \n";
        }
        oss << "};\n";

        oss << "\n";

        oss << "const DropSpawnPositions " << dropSpawnPositionsName << " = \n";
        oss << "{\n";
        oss << "    " << (dl_u16)dropSpawnPositions.spawnAreasCount << ", \n";
        oss << "    " << dropSpawnPositionsName << "_array, \n"; 
        oss << "};\n";

        std::string bankFolder = g_destinationPath;
        bankFolder += bankFolderNames[roomToBankFolderNameIndex[loop]];
        bankFolder += "\\";

        std::ofstream outFile(bankFolder + dropSpawnPositionsName + ".c");
        outFile << oss.str();        
    }
}

void saveResourcesHeader(Resources& resources)
{
    std::ostringstream oss;

    oss << "#ifndef RESOURCES_HEADER_INCLUDE_H\n";
    oss << "#define RESOURCES_HEADER_INCLUDE_H\n";
    oss << "\n";

    oss << "#include \"base_types.h\"\n";
    oss << "#include \"resource_types.h\"\n";
    oss << "#include \"custom_background_types.h\"\n";
    oss << "\n";

    oss << "extern const dl_u8 res_string_downland[14];\n";
    oss << "extern const dl_u8 res_string_writtenBy[12];\n";
    oss << "extern const dl_u8 res_string_michaelAichlmayer[18];\n";
    oss << "extern const dl_u8 res_string_copyright1983[15];\n";
    oss << "extern const dl_u8 res_string_spectralAssociates[20];\n";
    oss << "extern const dl_u8 res_string_licensedTo[13];\n";
    oss << "extern const dl_u8 res_string_tandyCorporation[18] ;\n";
    oss << "extern const dl_u8 res_string_allRightsReserved[20];\n";
    oss << "extern const dl_u8 res_string_onePlayer[11];\n";
    oss << "extern const dl_u8 res_string_twoPlayer[11];\n";
    oss << "extern const dl_u8 res_string_highScore[11];\n";
    oss << "extern const dl_u8 res_string_playerOne[11];\n";
    oss << "extern const dl_u8 res_string_playerTwo[11];\n";
    oss << "extern const dl_u8 res_string_pl1[4];\n";
    oss << "extern const dl_u8 res_string_pl2[4];\n";
    oss << "extern const dl_u8 res_string_getReadyPlayerOne[21];\n";
    oss << "extern const dl_u8 res_string_getReadyPlayerTwo[21];\n";
    oss << "extern const dl_u8 res_string_chamber[8];\n";
    oss << "\n";

    oss << "extern const PickupPosition res_roomPickupPositions[50];\n";
    oss << "extern const dl_u8 res_keyPickUpDoorIndexes[20];\n";
    oss << "extern const dl_u8 res_keyPickUpDoorIndexesHardMode[20];\n";
    oss << "extern const dl_u8 res_offsetsToDoorsAlreadyActivated[16];\n";
    oss << "extern const dl_u8 res_roomsWithBouncingBall[10];\n";

    oss << "\n";
    oss << "extern const RoomResources res_roomResources[NUM_ROOMS_PLUS_TITLESCREN];\n";

    oss << "\n";
    oss << "#endif\n";

    std::ofstream outFile(g_destinationPath + "resources.h");
    outFile << oss.str();
}

void saveResourcesSource(Resources& resources)
{
    std::ostringstream oss;

    oss << "#include \"base_types.h\"\n";
    oss << "#include \"resource_types.h\"\n";
    oss << "#include \"custom_background_types.h\"\n";
    oss << "\n";


    //oss << "extern const dl_u8 characterFont[273];\n";
    //oss << "\n";
    //oss << "extern const dl_u8 string_downland[14];\n";
    //oss << "extern const dl_u8 string_writtenBy[12];\n";
    //oss << "extern const dl_u8 string_michaelAichlmayer[18];\n";
    //oss << "extern const dl_u8 string_copyright1983[15];\n";
    //oss << "extern const dl_u8 string_spectralAssociates[20];\n";
    //oss << "extern const dl_u8 string_licensedTo[13];\n";
    //oss << "extern const dl_u8 string_tandyCorporation[18] ;\n";
    //oss << "extern const dl_u8 string_allRightsReserved[20];\n";
    //oss << "extern const dl_u8 string_onePlayer[11];\n";
    //oss << "extern const dl_u8 string_twoPlayer[11];\n";
    //oss << "extern const dl_u8 string_highScore[11];\n";
    //oss << "extern const dl_u8 string_playerOne[11];\n";
    //oss << "extern const dl_u8 string_playerTwo[11];\n";
    //oss << "extern const dl_u8 string_pl1[4];\n";
    //oss << "extern const dl_u8 string_pl2[4];\n";
    //oss << "extern const dl_u8 string_getReadyPlayerOne[21];\n";
    //oss << "extern const dl_u8 string_getReadyPlayerTwo[21];\n";
    //oss << "extern const dl_u8 string_chamber[8];\n";
    //oss << "\n";
    //oss << "extern const dl_u8 bitShiftedSprite_player[1920];\n";
    //oss << "extern const dl_u8 bitShiftedSprite_ball[192];\n";
    //oss << "extern const dl_u8 bitShiftedSprite_bird[144];\n";
    //oss << "extern const dl_u8 bitShiftedSprite_door[192];\n";
    //oss << "extern const dl_u8 bitShiftedSprite_playerCollisionMasks[600];\n";
    //oss << "extern const dl_u8 bitShiftedSprite_playerSplat[108];\n";
    //oss << "\n";
    //oss << "extern const dl_u8 diamondSprite[20];\n";
    //oss << "extern const dl_u8 moneyBagSprite[20];\n";
    //oss << "extern const dl_u8 keySprite[20];\n";
    //oss << "\n";
    //oss << "extern const dl_u8 dropSprite[48];\n";
    //oss << "\n";
    //oss << "extern const PickupPosition roomPickupPositions[50];\n";
    //oss << "extern const dl_u8 keyPickUpDoorIndexes[20];\n";
    //oss << "extern const dl_u8 keyPickUpDoorIndexesHardMode[20];\n";
    //oss << "extern const dl_u8 offsetsToDoorsAlreadyActivated[16];\n";
    //oss << "extern const dl_u8 roomsWithBouncingBall[10];\n";
    //oss << "\n";
    //oss << "\n";

    saveString(resources.text_downland, "downland", oss);
    saveString(resources.text_writtenBy, "writtenBy", oss);
    saveString(resources.text_michaelAichlmayer, "michaelAichlmayer", oss);
    saveString(resources.text_copyright1983, "copyright1983", oss);
    saveString(resources.text_spectralAssociates, "spectralAssociates", oss);
    saveString(resources.text_licensedTo, "licensedTo", oss);
    saveString(resources.text_tandyCorporation, "tandyCorporation", oss);
    saveString(resources.text_allRightsReserved, "allRightsReserved", oss);
    saveString(resources.text_onePlayer, "onePlayer", oss);
    saveString(resources.text_twoPlayer, "twoPlayer", oss);
    saveString(resources.text_highScore, "highScore", oss);
    saveString(resources.text_playerOne, "playerOne", oss);
    saveString(resources.text_playerTwo, "playerTwo", oss);
    saveString(resources.text_pl1, "pl1", oss);
    saveString(resources.text_pl2, "pl2", oss);
    saveString(resources.text_getReadyPlayerOne, "getReadyPlayerOne", oss);
    saveString(resources.text_getReadyPlayerTwo, "getReadyPlayerTwo", oss);
    saveString(resources.text_chamber, "chamber", oss);
    oss << "\n";
    oss << "\n";

    // pick up positions
    oss << "// pick up positions (x: 0 - 127, y: 0 - 191)\n";
    const PickupPosition* roomPickupPositions = resources.roomPickupPositions;
    oss << "const PickupPosition res_roomPickupPositions[" << std::dec << NUM_ROOMS * NUM_PICKUPS_PER_ROOM << "] = \n";
    oss << "{\n";
    for (int loop = 0; loop < NUM_ROOMS * NUM_PICKUPS_PER_ROOM; loop++)
    {
        oss << "    { ";
        oss << (dl_u16)roomPickupPositions->y << ", ";
        oss << (dl_u16)roomPickupPositions->x;
        oss << " },\n";

        roomPickupPositions++;
    }
    oss << "};\n";
    oss << "\n";

    // pick up door indexes
    oss << "// pick up door indexes\n";
    const dl_u8* keyPickUpDoorIndexes = resources.keyPickUpDoorIndexes;
    oss << "const dl_u8 res_keyPickUpDoorIndexes[20] = { ";

    for (int loop = 0; loop < 20; loop++)
    {
        oss << (dl_u16)*keyPickUpDoorIndexes << ", ";
        keyPickUpDoorIndexes++;
    }
    oss << "};\n";
    oss << "\n";

    // pick up door indexes
    oss << "// pick up door indexes (hard mode)\n";
    keyPickUpDoorIndexes = resources.keyPickUpDoorIndexesHardMode;
    oss << "const dl_u8 res_keyPickUpDoorIndexesHardMode[20] = { ";

    for (int loop = 0; loop < 20; loop++)
    {
        oss << (dl_u16)*keyPickUpDoorIndexes << ", ";
        keyPickUpDoorIndexes++;
    }
    oss << "};\n";
    oss << "\n";

    // offests to doors alread activated
    oss << "// offests to doors alread activated\n";
    const dl_u8* offsetsToDoorsAlreadyActivated = resources.offsetsToDoorsAlreadyActivated;
    oss << "const dl_u8 res_offsetsToDoorsAlreadyActivated[16] = { ";

    for (int loop = 0; loop < 16; loop++)
    {
        oss << (dl_u16)*offsetsToDoorsAlreadyActivated << ", ";
        offsetsToDoorsAlreadyActivated++;
    }
    oss << "};\n";
    oss << "\n";

    // rooms with the bouncing ball
    oss << "// rooms with the bouncing ball\n";
    const dl_u8* roomsWithBouncingBall = resources.roomsWithBouncingBall;
    oss << "const dl_u8 res_roomsWithBouncingBall[10] = { ";

    while (*roomsWithBouncingBall != 0xff)
    {
        oss << (dl_u16)*roomsWithBouncingBall << ", ";
        roomsWithBouncingBall++;
    }
    oss << "0xff };\n";
    oss << "\n";

    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
    {
        oss << "extern const dl_u8 " << roomNames[loop] << "_cleanBackground[" << FRAMEBUFFER_PITCH * FRAMEBUFFER_HEIGHT << "];\n";
        oss << "extern const dl_u8 " << roomNames[loop] << "_tileMap[32 * 24];\n";

        oss << "const BackgroundData " << roomNames[loop] << "_customBackgroundData = "
            << "{ " 
            << roomNames[loop] << "_cleanBackground"
            << ", "
            << roomNames[loop] << "_tileMap"
            << "};\n";


        oss << "\n";
    }
    oss << "\n";

    oss << "extern const dl_u8 getReadyScreen_cleanBackground[" << FRAMEBUFFER_PITCH * FRAMEBUFFER_HEIGHT << "];\n";
    oss << "\n";

    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
    {
        std::string dropSpawnPositionsName = roomNames[loop];
        dropSpawnPositionsName += "_dropSpawnPositions";

        const DropSpawnPositions& dropSpawnPositions = resources.roomResources[loop].dropSpawnPositions;

        oss << "const DropSpawnArea " << dropSpawnPositionsName << "_array[" << (dl_u16)dropSpawnPositions.spawnAreasCount << "] = \n";  
        oss << "{\n";
        for (int innerLoop = 0; innerLoop < dropSpawnPositions.spawnAreasCount; innerLoop++)
        {
            oss << "    { ";
            oss << (dl_u16)dropSpawnPositions.dropSpawnAreas[innerLoop].dropSpawnPointsCount << ", ";
            oss << (dl_u16)dropSpawnPositions.dropSpawnAreas[innerLoop].y << ", ";
            oss << (dl_u16)dropSpawnPositions.dropSpawnAreas[innerLoop].x;
            oss << " }, \n";
        }
        oss << "};\n";
        oss << "\n";
    }

    for (int loop = 0; loop < NUM_ROOMS; loop++)
    {
        const DoorInfoData& doorInfoData = resources.roomResources[loop].doorInfoData;

        oss << "const DoorInfo doorInfo" << (dl_u16)loop << "_array[" << (dl_u16)doorInfoData.drawInfosCount << "] = \n";
        oss << "{ \n";
        for (int innerLoop = 0; innerLoop < doorInfoData.drawInfosCount; innerLoop++)
        {
            const DoorInfo& doorInfo = doorInfoData.doorInfos[innerLoop];

            oss << "    { ";
            oss << (dl_u16)doorInfo.y << ", ";
            oss << (dl_u16)doorInfo.x << ", ";
            oss << (dl_u16)doorInfo.yLocationInNextRoom << ", ";
            oss << (dl_u16)doorInfo.xLocationInNextRoom << ", ";
            oss << (dl_u16)doorInfo.nextRoomNumber << ", ";
            oss << (dl_u16)doorInfo.globalDoorIndex << " },\n";
        }
        oss << "};\n";
        oss << "\n";
    }

	//oss << "const Resources resources = \n";
	//oss << "{\n";
    //oss << "    characterFont,\n";
    //oss << "\n";

    //oss << "    // strings\n";
    //oss << "    string_downland,\n";
    //oss << "    string_writtenBy,\n";
    //oss << "    string_michaelAichlmayer,\n";
    //oss << "    string_copyright1983,\n";
    //oss << "    string_spectralAssociates,\n";
    //oss << "    string_licensedTo,\n";
    //oss << "    string_tandyCorporation,\n";
    //oss << "    string_allRightsReserved,\n";
    //oss << "    string_onePlayer,\n";
    //oss << "    string_twoPlayer,\n";
    //oss << "    string_highScore,\n";
    //oss << "    string_playerOne,\n";
    //oss << "    string_playerTwo,\n";
    //oss << "    string_pl1,\n";
    //oss << "    string_pl2,\n";
    //oss << "    string_getReadyPlayerOne,\n";
    //oss << "    string_getReadyPlayerTwo,\n";
    //oss << "    string_chamber,\n";
    //oss << "\n";

    //oss << "    // sprites\n";
    //oss << "    NULL, // sprites_player\n";
    //oss << "    NULL, // collisionmasks_player,\n";
    //oss << "    NULL, // sprites_bouncyBall,\n";
    //oss << "    NULL, // sprites_bird,\n";
    //oss << "    moneyBagSprite,\n";
    //oss << "    diamondSprite,\n";
    //oss << "    keySprite,\n";
    //oss << "    NULL, // sprite_playerSplat,\n";
    //oss << "    NULL, // sprite_door,\n";
    //oss << "    dropSprite,\n";
    //oss << "\n";

    //oss << "    // bit shifted sprites\n";
    //oss << "    bitShiftedSprite_player,\n";
    //oss << "    bitShiftedSprite_playerCollisionMasks,\n";
    //oss << "    bitShiftedSprite_ball,\n";
    //oss << "    bitShiftedSprite_bird,\n";
    //oss << "    bitShiftedSprite_playerSplat,\n";
    //oss << "    bitShiftedSprite_door,\n";
    //oss << "\n";
    //
    //oss << "    { diamondSprite, moneyBagSprite, keySprite },\n";
    //oss << "\n";

    oss << "const RoomResources res_roomResources[NUM_ROOMS_PLUS_TITLESCREN] = \n";
    oss << "{\n";

    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
    {
        oss << "    { ";

        oss << "(const dl_u8*)&" << roomNames[loop] << "_customBackgroundData, ";

        std::string dropSpawnPositionsName = roomNames[loop];
        dropSpawnPositionsName += "_dropSpawnPositions_array";
        const DropSpawnPositions& dropSpawnPositions = resources.roomResources[loop].dropSpawnPositions;

        oss << "{ " 
            << (dl_u16)dropSpawnPositions.spawnAreasCount
            << ", "
            << dropSpawnPositionsName
            << " }, ";

        if (loop < NUM_ROOMS)
        {
            const DoorInfoData& doorInfoData = resources.roomResources[loop].doorInfoData;

            oss << "{ " 
                << (dl_u16)doorInfoData.drawInfosCount
                << ", "
                << "doorInfo" << (dl_u16)loop << "_array"
                << " } },\n";
        }
        else
        {
            oss << "{ 0, NULL } },\n";
        }
    }

    oss << "};\n";
    oss << "\n";

    //oss << "    roomPickupPositions,            // 50 items\n";
    //oss << "    keyPickUpDoorIndexes,           // 20 items\n";
    //oss << "    keyPickUpDoorIndexesHardMode,   // 20 items\n";
    //oss << "    offsetsToDoorsAlreadyActivated, // 16 items\n";
    //oss << "    roomsWithBouncingBall,          // 10 items\n";
    //
    //oss << "\n";
    //
	//oss << "};\n\n";

    std::ofstream outFile(g_destinationPath + "resources.c");
    outFile << oss.str();
}

void createFolder(std::string& folder)
{
    if (!std::filesystem::exists(folder)) 
    {
        // create the generated folder
        std::filesystem::create_directories(folder); 
    }
    else
    {
        // clear everything in the folder
        for (const auto& entry : std::filesystem::directory_iterator(folder)) 
        {
            std::filesystem::remove_all(entry);
        }
    }
}

int main()
{
    Resources resources;

    //std::filesystem::path cwd = std::filesystem::current_path();

    createFolder(g_destinationPath); 

    for (int loop = 0; loop < bankFolderNameCount; loop++)
    {
        std::string path = g_destinationPath;
        path += bankFolderNames[loop];
        createFolder(path); 
    }

    std::cout << "**** DLExporter Master System START\n";

    auto downland_rom = load_binary_file("downland.rom");

    ResourceLoaderBuffer_Init(downland_rom.data(), DOWNLAND_ROM_SIZE, &resources);

    dl_u8 background[FRAMEBUFFER_PITCH * FRAMEBUFFER_HEIGHT];
    dl_u8 background8bpp[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

    std::vector<Tile> tileSet;
    std::vector<TileMap> tileMaps;


    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
    {
        drawBackground(&resources.roomResources[loop].backgroundDrawData, 
				       &resources,
				       background);

        convert1bppImageTo8bppCrtEffectImage(background,
                                             background8bpp,
                                             FRAMEBUFFER_WIDTH,
                                             FRAMEBUFFER_HEIGHT,
                                             CrtColor_Blue);

        if (loop == TITLESCREEN_ROOM_INDEX)
        {
	        // title screen text
	        drawText(resources.text_downland, resources.characterFont, background, 0x03c9); // 0x07c9 original coco mem location
	        drawText(resources.text_writtenBy, resources.characterFont, background, 0x050a); // 0x090A original coco mem location
	        drawText(resources.text_michaelAichlmayer, resources.characterFont, background, 0x647); // 0x0A47 original coco mem location
	        drawText(resources.text_copyright1983, resources.characterFont, background, 0x789); // 0x0B89 original coco mem location
	        drawText(resources.text_spectralAssociates, resources.characterFont, background, 0x8c6); // 0x0CC6 original coco mem location
	        drawText(resources.text_licensedTo, resources.characterFont, background, 0xa0a); // 0x0E0A original coco mem location
	        drawText(resources.text_tandyCorporation, resources.characterFont, background, 0xb47); // 0x0F47 original coco mem location
	        drawText(resources.text_allRightsReserved, resources.characterFont, background, 0xc86); // 0x1086 original coco mem location
	        drawText(resources.text_onePlayer, resources.characterFont, background, 0xf05); // 0x1305 original coco mem location
	        drawText(resources.text_twoPlayer, resources.characterFont, background, 0xf11); // 0x1311 original coco mem location
	        drawText(resources.text_highScore, resources.characterFont, background, 0x118b); // 0x158B original coco mem location
	        drawText(resources.text_playerOne, resources.characterFont, background, 0x1406); // 0x1806 original coco mem location
	        drawText(resources.text_playerTwo, resources.characterFont, background, 0x1546); // 0x1946 original coco mem location
        }

        saveCleanBackground(background, loop);
        TileMap tileMap;

        buildTileMap(background8bpp, tileMap, tileSet);

        tileMaps.push_back(tileMap);
    }
    
    // get ready screen
    drawBackground(&resources.roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData, 
				   &resources,
				   background);

	drawText(resources.text_getReadyPlayerOne, resources.characterFont, background, 0x0b66);

    saveCleanBackground(background, GET_READY_ROOM_INDEX);
    

    const int _2bppTileWidth = TILE_WIDTH >> 2;
    const int backgroundChrTileCount = 256;
    const int spriteChrTileCount = 256;
    const int chrTileCount = backgroundChrTileCount + spriteChrTileCount;
    const int chrBufferSize = chrTileCount * _2bppTileWidth * TILE_HEIGHT;

    dl_u8 chrBuffer[chrBufferSize];
    memset(chrBuffer, 0, sizeof(chrBuffer));

    dl_u8* chrBufferRunner = chrBuffer;

    // export the sprite tiles
    chrBufferRunner = saveSpriteToChr(resources.sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT, chrBufferRunner);
	chrBufferRunner = saveSpriteToChr(resources.sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT, chrBufferRunner);
	chrBufferRunner = saveSpriteToChr(resources.sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT, chrBufferRunner, true);
	chrBufferRunner = saveSpriteToChr(resources.sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT, chrBufferRunner, true);
	chrBufferRunner = saveSpriteToChr(resources.sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, chrBufferRunner);
	chrBufferRunner = saveSpriteToChr(resources.sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, chrBufferRunner);
	chrBufferRunner = saveSpriteToChr(resources.sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, chrBufferRunner);
	chrBufferRunner = saveSpriteToChr(resources.sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1, chrBufferRunner);
    chrBufferRunner = saveCursorToChr(chrBufferRunner);

    dl_u8* regenSprite = saveRegenSpriteToChr(resources.sprites_player, &chrBufferRunner);
    chrBufferRunner = saveRegenLivesSpriteToChr(regenSprite, chrBufferRunner);

    delete [] regenSprite;

    chrBufferRunner = saveSplatSpriteToChr(resources.sprite_playerSplat, chrBufferRunner);
    chrBufferRunner = saveSprite16ClippedToChr(resources.sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT, chrBufferRunner);


    // start at the sprite tiles location
    chrBufferRunner = chrBuffer + (sizeof(chrBuffer) / 2);

    // export the background tiles
    chrBufferRunner = saveTileSetToChr(tileSet, chrBufferRunner);
    chrBufferRunner = saveCharacterFontToChr(resources.characterFont, chrBufferRunner);


    std::string chrPath = g_destinationPath + "downlandTileset.chr";
    FILE* file;
    fopen_s(&file, chrPath.c_str(), "w");
    fwrite(chrBuffer, sizeof(chrBuffer), 1, file);
    fclose(file);

    saveTileMapSource(tileMaps);
    saveResourcesHeader(resources);
    saveResourcesSource(resources);

    std::cout << "**** DLExporter END\n";
}
