#ifndef DRAW_SPRITE_256_ASM_INCLUDE_H
#define DRAW_SPRITE_256_ASM_INCLUDE_H

extern void drawRow16x16MaskedFastAsm256(/* dl_u8* dest, const dl_u8* src */);
extern void drawRow16x10MaskedFastAsm256(/* dl_u8* dest, const dl_u8* src */);
extern void drawText8x7UnconditionalAsm256(/* dl_u8* dest, const dl_u8* src */);

#endif
