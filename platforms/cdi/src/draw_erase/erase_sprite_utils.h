#ifndef ERASE_SPRITE_UTILS_INCLUDE_H
#define ERASE_SPRITE_UTILS_INCLUDE_H

#include "base_types.h"

#define SCREEN_OFFSET_X 64
#define SCREEN_OFFSET_Y 24

// Erase by specialized width
void eraseSprite16(dl_u16 x, dl_u16 y, dl_u8 height);
void eraseSprite24(dl_u16 x, dl_u16 y, dl_u8 height);
void eraseSprite8(dl_u16 x, dl_u16 y, dl_u8 height);

// Specialized erase for drops that erases 4x6 for it's 16x6 srite.
void eraseSpriteDrop(dl_u16 x, dl_u16 y, dl_u8 height);

#endif
