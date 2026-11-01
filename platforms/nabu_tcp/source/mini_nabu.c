#include <stdint.h>
#include <arch/nabu.h>
#include "mini_nabu.h"

void NABU_DisableInterrupts(void) __naked
{
    __asm
        di
        ret
    __endasm;
}

void NABU_EnableInterrupts(void) __naked
{
    __asm
        ei
        ret
    __endasm;
}

void ayWrite(uint8_t reg, uint8_t val)
{
    IO_AYLATCH = reg;
    IO_AYDATA = val;
}

uint8_t ayRead(uint8_t reg)
{
    IO_AYLATCH = reg;
    return IO_AYDATA;
}