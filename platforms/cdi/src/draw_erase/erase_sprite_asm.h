#ifndef ERASE_SPRITE_ASM_INCLUDE_H
#define ERASE_SPRITE_ASM_INCLUDE_H

extern void eraseRestoreRow16x16Asm(/* dl_u8* dest (D0), const dl_u8* src (D1) */);
extern void eraseRestoreRow16x10Asm(/* dl_u8* dest (D0), const dl_u8* src (D1) */);
extern void eraseRestoreRow16x8Asm(/* dl_u8* dest (D0), const dl_u8* src (D1) */);
extern void eraseRestoreRow16x6Asm(/* dl_u8* dest (D0), const dl_u8* src (D1) */);
extern void eraseRestoreRow4x6Asm(/* dl_u8* dest (D0), const dl_u8* src (D1) */);

#endif
