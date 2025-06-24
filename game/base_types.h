#ifndef BASE_TYPES_INCLUDE_H
#define BASE_TYPES_INCLUDE_H

typedef signed char dl_s8;
typedef unsigned char dl_u8;

typedef signed short dl_s16;
typedef unsigned short dl_u16;

typedef signed int dl_s32;
typedef unsigned int dl_u32;

#define BOOL dl_u8
#define TRUE 1
#define FALSE 0

#undef NULL
#define NULL 0

#define UNUSED(x) ((void)x)

#endif