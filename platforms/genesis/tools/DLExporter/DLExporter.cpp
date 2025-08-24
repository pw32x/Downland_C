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

#include "lodepng.h"

std::string g_resPath = "res\\";

extern "C"
{
#include "base_types.h"
#include "base_defines.h"
#include "resource_types.h"
#include "resource_loader_buffer.h"
#include "draw_utils.h"
#include "drops_manager.h"
#include "rooms\chambers.h"

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

#define TILE_WIDTH 8
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

void save_png_8bpp(const dl_u8* background,
                   unsigned width, 
                   unsigned height,
                   const std::string& filename)
{
    LodePNGState state;
    lodepng_state_init(&state);

    state.encoder.auto_convert = 0;

    // Set to indexed color
    state.info_raw.colortype = LCT_PALETTE;
    state.info_raw.bitdepth = 8;
    state.info_png.color.colortype = LCT_PALETTE;
    state.info_png.color.bitdepth = 8;

    lodepng_palette_add(&state.info_png.color,   0,   0,   0, 255); // index 0: black
    lodepng_palette_add(&state.info_png.color,   0,   0, 255, 255); // index 1: blue
    lodepng_palette_add(&state.info_png.color, 255, 165,   0, 255); // index 2: orange
    lodepng_palette_add(&state.info_png.color, 255, 255, 255, 255); // index 3: white

    lodepng_palette_add(&state.info_raw,   0,   0,   0, 255); // index 0: black
    lodepng_palette_add(&state.info_raw,   0,   0, 255, 255); // index 1: blue
    lodepng_palette_add(&state.info_raw, 255, 165,   0, 255); // index 2: orange
    lodepng_palette_add(&state.info_raw, 255, 255, 255, 255); // index 3: white

    // Encode
    dl_u8* png;
    size_t pngSize;
    unsigned error = lodepng_encode(&png, &pngSize, background, width, height, &state);

    if(error)
    {
        throw std::exception("lodepng_encode failed.");
    }

    // Save to file
    error = lodepng_save_file(png, pngSize, filename.c_str());

    if (error) 
    {
        throw std::exception("lodepng_save_file failed.");
    }

    lodepng_state_cleanup(&state);
}

void saveTileSetToPng(const TileSet& tileSet)
{
    dl_u8* tileSetBitmap = new dl_u8[tileSet.size() * TILE_SIZE];

    dl_u16 counter = 0;
    for (auto& tile : tileSet)
    {
        memcpy(tileSetBitmap + (counter * TILE_SIZE), tileSet[counter].data(), TILE_SIZE);
        counter++;
    }

    save_png_8bpp(tileSetBitmap, 
                  TILE_WIDTH,
                  TILE_HEIGHT * static_cast<dl_u16>(tileSet.size()),
                  g_resPath + "backgroundTileset.png");


    delete [] tileSetBitmap;
}



void saveResFile()
{
    std::ostringstream oss;

    oss << "TILESET characterFontTileset \"characterFontTileset.png\" BEST NONE\n";
    oss << "TILESET dropTileset \"dropTileset.png\" BEST NONE\n";
    oss << "SPRITE playerTileset \"playerTileset.png\" 2 2 NONE 0\n";
    oss << "SPRITE regenTileset \"regenTileset.png\" 2 2 NONE 0\n";
    oss << "TILESET ballTileset \"ballTileset.png\" BEST NONE\n";
    oss << "TILESET birdTileset \"birdTileset.png\" BEST NONE\n";
    oss << "SPRITE keyTileset \"keyTileset.png\" 2 2 NONE 0\n";
    oss << "SPRITE diamondTileset \"diamondTileset.png\" 2 2 NONE 0\n";
    oss << "SPRITE moneyBagTileset \"moneyBagTileset.png\" 2 2 NONE 0\n";
    oss << "SPRITE doorTileset \"doorTileset.png\" 2 2 NONE 0\n";
    oss << "SPRITE cursorTileset \"cursorTileset.png\" 1 1 NONE 0\n";
    oss << "SPRITE playerSplatTileset \"playerSplatTileset.png\" 3 2 NONE 0 NONE NONE\n";
    oss << "SPRITE playerLivesTileset \"playerLivesTileset.png\" 2 1 NONE 0 NONE NONE\n";
    oss << "SPRITE playerLivesRegenTileset \"playerLivesRegenTileset.png\" 2 1 NONE 0 NONE NONE\n";
    oss << "TILESET backgroundTileset \"backgroundTileset.png\" BEST NONE\n";
    oss << "TILESET transitionTileset \"transitionTileset.png\" BEST NONE\n";

    std::ofstream outFile(g_resPath + "tileset.res");
    outFile << oss.str();
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
"titleScreen"
};

void saveTileMapSource(const std::vector<TileMap>& tileMaps)
{
    std::ostringstream oss;

    oss << "#include \"base_types.h\"\n";
    oss << "\n";

    dl_u8 counter = 0;
    for (auto& tileMap : tileMaps)
    {
        oss << "dl_u16 " << roomNames[counter] << "TileMap[] = \n";
        oss << "{\n";

        for (int loopy = 0; loopy < TILE_MAP_HEIGHT; loopy++)
        {
            oss << "    ";

            for (int loopx = 0; loopx < TILE_MAP_WIDTH; loopx++)
            {
                dl_u16 tileIndex = tileMap[loopx + (loopy * TILE_MAP_WIDTH)];

                oss << tileIndex << ", ";
            }

            oss << "\n";
        }

        oss << "};\n";
        oss << "\n";

        counter++;
    }

    oss << "\n";
    oss << "dl_u16* roomTileMaps[" << NUM_ROOMS_PLUS_TITLESCREN << "] = \n";
    oss << "{\n";
    for (int loop = 0; loop < NUM_ROOMS_PLUS_TITLESCREN; loop++)
        oss << "    " << roomNames[loop] << "TileMap,\n";
    oss << "};\n";


    std::ofstream outFile(g_resPath + "tileMaps.c");
    outFile << oss.str();
}

void saveTileMapHeader()
{
    std::ostringstream oss;

    oss << "#ifndef TILEMAPS_HEADER_INCLUDE_H\n";
    oss << "#define TILEMAPS_HEADER_INCLUDE_H\n";
    oss << "\n";
    oss << "extern dl_u16* roomTileMaps[" << NUM_ROOMS_PLUS_TITLESCREN << "];\n";
    oss << "\n";
    oss << "#endif\n";

    std::ofstream outFile(g_resPath + "tileMaps.h");
    outFile << oss.str();
}

void saveCharacterFont(const dl_u8* characterFont)
{
#define DESTINATION_FONT_HEIGHT 8

    dl_u8* destinationFont = new dl_u8[CHARACTER_FONT_WIDTH * DESTINATION_FONT_HEIGHT * CHARACTER_FONT_COUNT];
    memset(destinationFont, 0, CHARACTER_FONT_WIDTH * DESTINATION_FONT_HEIGHT * CHARACTER_FONT_COUNT);

    dl_u8* destinationFontRunner = destinationFont;

    for (int characterLoop = 0; characterLoop < CHARACTER_FONT_COUNT; characterLoop++)
    {
        for (int loopy = 0; loopy < CHARACTER_FONT_HEIGHT; loopy++)
        {
            dl_u8 characterRow = characterFont[loopy];

            for (int loopx = 0; loopx < CHARACTER_FONT_WIDTH; loopx++)
            {
                dl_u8 value = characterRow & 1;
                characterRow >>= 1; // next bit
                destinationFontRunner[((CHARACTER_FONT_WIDTH - 1) - loopx) + (loopy * CHARACTER_FONT_WIDTH)] = value;
            }
        }


        characterFont += CHARACTER_FONT_HEIGHT; // move to next character

        destinationFontRunner += CHARACTER_FONT_WIDTH * DESTINATION_FONT_HEIGHT; // destination height
    }

    save_png_8bpp(destinationFont, 
                  CHARACTER_FONT_WIDTH,
                  DESTINATION_FONT_HEIGHT * CHARACTER_FONT_COUNT,
                  g_resPath + "characterFontTileset.png");

    delete [] destinationFont;
}

void saveCursor()
{
    dl_u8 cursor8bpp[8 * 8];
    memset(cursor8bpp, 0, 8*8);
    memset(cursor8bpp, 3, 8);

    save_png_8bpp(cursor8bpp, 
                  8,
                  8,
                  g_resPath + "cursorTileset.png");
}

dl_u8* saveRegenSprite(const dl_u8* playerSprite)
{
    const int numFrames = 8;
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

        // move to next frame
        sprite8bppRunner += destinationFrameSize; 
    }

    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * (numFrames * 2),
                  g_resPath + "regenTileset.png");

    delete [] regenBuffer;

    return sprite8bpp;
}

void saveRegenLivesSprite(dl_u8* regenSprite)
{
    const int numFrames = 16;
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

        regenSprite += regenSpriteFrameSize;
        regenIconBufferRunner += regenIconFrameSize;
    }

    save_png_8bpp(regenIconBuffer, 
                  regenWidth,
                  iconImageHeight * numFrames,
                  g_resPath + "playerLivesRegenTileset.png");

    delete [] regenIconBuffer;
}

void saveSplatSprite(const dl_u8* splatSprite)
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

    dl_u8* secondFrame = sprite8bpp + destinationFrameSize;

    // write it again but on the next frame
    convert1bppImageTo8bppCrtEffectImage(splatSprite,
                                         secondFrame,
                                         width,
                                         height,
                                         CrtColor::CrtColor_Blue);

    // clear the top five rows
    memset(secondFrame, 0, 24 * 5);

    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * numFrames,
                  g_resPath + "playerSplatTileset.png");

    delete [] sprite8bpp;
}


void saveSprite16(const dl_u8* sprite, 
                  dl_u8 width, 
                  dl_u8 height, 
                  dl_u8 numFrames, 
                  const std::string& name)
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

        // move to next frame
        sprite += spriteFrameSizeInBytes;
        sprite8bppRunner += destinationFrameSize; 
    }

    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * numFrames,
                  g_resPath + name + ".png");

    delete [] sprite8bpp;
}


void saveSprite16Clipped(const dl_u8* sprite, 
                         dl_u8 width, 
                         dl_u8 height, 
                         dl_u8 clipHeight,
                         dl_u8 numFrames, 
                         const std::string& name)
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

        // move to next frame
        sprite += spriteFrameSizeInBytes;
        sprite8bppRunner += destinationFrameSize; 
    }

    save_png_8bpp(sprite8bpp, 
                  destinationWidth,
                  destinationHeight * numFrames,
                  g_resPath + name + ".png");

    delete [] sprite8bpp;
}

void saveTransitionTileset()
{
    const dl_u8 transparentColor = 0;
    const dl_u8 lineColor = 1;
    const dl_u8 opaqueColor = 4;

    const dl_u8 numAnimatedFrames = 8;
    const dl_u8 numTotalTiles = numAnimatedFrames + 1; // 8 animated frames and 1 black frame
    const dl_u16 bufferSize = TILE_SIZE * numTotalTiles;

    dl_u8 tilesetBuffer[bufferSize];
    memset(tilesetBuffer, opaqueColor, bufferSize);

    dl_u8* tilesetBufferRunner = tilesetBuffer;

    // first tile is completely black
    memset(tilesetBuffer, opaqueColor, TILE_SIZE);

    tilesetBufferRunner += TILE_SIZE;

    // next 8 tiles are animated with the transition line
    for (int loop = 0; loop < numAnimatedFrames; loop++)
    {
        for (int row = 0; row < TILE_HEIGHT; row++)
        {
            if (row < loop)
            {
                memset(tilesetBufferRunner, transparentColor, TILE_WIDTH);
            }
            else if (row > loop)
            {
                memset(tilesetBufferRunner, opaqueColor, TILE_WIDTH);
            }
            else if (row == loop)
            {
                memset(tilesetBufferRunner, lineColor, TILE_WIDTH);
            }
            
            tilesetBufferRunner += TILE_WIDTH; // next row
        }
    }

    save_png_8bpp(tilesetBuffer, 
                  TILE_WIDTH,
                  TILE_HEIGHT * numTotalTiles,
                  g_resPath + "transitionTileset.png");
}

int main()
{
    Resources resources;

    std::cout << "**** DLExporter START\n";

    auto downland_rom = load_binary_file(g_resPath + "downland.rom");

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

        TileMap tileMap;

        buildTileMap(background8bpp, tileMap, tileSet);

        tileMaps.push_back(tileMap);
    }

    saveCharacterFont(resources.characterFont);
    saveSprite16(resources.sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT, "dropTileset");   
	saveSprite16(resources.sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT, "playerTileset");
	//(dl_u8*)&cursorSpriteRaw, 8, 1, 1, tileIndex);
	saveSprite16(resources.sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT, "ballTileset");
	saveSprite16(resources.sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT, "birdTileset");
	saveSprite16(resources.sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, "keyTileset");
	saveSprite16(resources.sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, "diamondTileset");
	saveSprite16(resources.sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, "moneyBagTileset");
	saveSprite16(resources.sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1, "doorTileset");
	//buildEmptySpriteResource(&regenSprite, &g_16x16SpriteAttributes, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 1, tileIndex);

    saveCursor();
    dl_u8* regenSprite = saveRegenSprite(resources.sprites_player);
    saveRegenLivesSprite(regenSprite);

    delete [] regenSprite;

    saveSplatSprite(resources.sprite_playerSplat);
    saveSprite16Clipped(resources.sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT, "playerLivesTileset");
    saveTransitionTileset();
    saveTileSetToPng(tileSet);
    saveTileMapSource(tileMaps);
    saveTileMapHeader();
    saveResFile();

    std::cout << "**** DLExporter END\n";
}
