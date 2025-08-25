#ifndef MAP_TYPES_INCLUDE_H
#define MAP_TYPES_INCLUDE_H

typedef struct
{
    const unsigned char* const tiles;
    const unsigned short* const metatile_lut;
    const char numTiles;
    const char numMetatiles;
} Tileset;

typedef struct
{
    const unsigned short* const metatileMap;
    const Tileset* const tileset;
    unsigned char mapWidth;
    unsigned char mapHeight;
} Map;

#endif