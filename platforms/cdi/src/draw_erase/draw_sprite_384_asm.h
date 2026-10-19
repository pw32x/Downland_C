#ifndef DRAW_SPRITE_384_ASM_INCLUDE_H
#define DRAW_SPRITE_384_ASM_INCLUDE_H

extern void drawRow16x16MaskedFastAsm(/* dl_u8* dest, const dl_u8* src */);
extern void drawRow16x10MaskedFastAsm(/* dl_u8* dest, const dl_u8* src */);
extern void drawRow16x8MaskedFastAsm(/* dl_u8* dest, const dl_u8* src */);
extern void drawRow16x6MaskedFastAsm(/* dl_u8* dest, const dl_u8* src */);
extern void drawRow4x6MaskedFastAsm(/* dl_u8* dest, const dl_u8* src */);
extern void drawOpaqueRow16x16Asm(/* dl_u8* dest, const dl_u8* src */);
extern void drawOpaqueRow16x10Asm(/* dl_u8* dest, const dl_u8* src */);
extern void drawOpaqueRow16x7Asm(/* dl_u8* dest, const dl_u8* src */);
extern void drawText8x7UnconditionalAsm(/* dl_u8* dest, const dl_u8* src */);

#endif
