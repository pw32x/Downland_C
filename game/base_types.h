#ifndef BASE_TYPES_INCLUDE_H
#define BASE_TYPES_INCLUDE_H

typedef signed char s8;
typedef unsigned char byte;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

#define BOOL u8
#define TRUE 1
#define FALSE 0

#undef NULL
#define NULL 0

#define UNUSED(x) ((void)x)

#endif