#ifndef ERASE_SYSTEM_INCLUDE_H
#define ERASE_SYSTEM_INCLUDE_H

#include "base_types.h"

typedef void (*EraseFunc)(dl_u16 x, dl_u16 y, dl_u8 height);

void initEraseList(void);
void addToEraseList(dl_u16 x, dl_u16 y, dl_u8 height, EraseFunc eraseFunc);
void addToOtherEraseList(dl_u16 x, dl_u16 y, dl_u8 height, EraseFunc eraseFunc);
void processEraseList(void);

#endif
