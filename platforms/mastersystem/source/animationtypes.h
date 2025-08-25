#ifndef ANIMATION_TYPES_INCLUDE_H
#define ANIMATION_TYPES_INCLUDE_H


typedef struct
{
    char xOffset;
    char yOffset;
    unsigned char tileIndex;
} AnimationSprite;

typedef struct
{
    const AnimationSprite* const sprites;
    unsigned char numSprites;
    unsigned char frameTime;
} AnimationFrame;

typedef struct
{
    const AnimationFrame** const frames;
    const unsigned char* const tileData;

    unsigned char numFrames;
    unsigned char pixelWidth;
    unsigned char pixelHeight;
    unsigned char maxTilesPerFrame;

    unsigned char totalTileCount;
	unsigned short totalAnimationTime;

} Animation;

#endif