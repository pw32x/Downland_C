#ifndef MINI_NABU_LIB_H
#define MINI_NABU_LIB_H

#include <stdint.h>
#include <arch/nabu.h>

/* AY-3-8910 I/O port registers. */
#define IOPORTA             14
#define IOPORTB             15

/* HCCA interrupt mask bits in AY register 14. */
#define INT_MASK_HCCARX     0x80
#define INT_MASK_HCCATX     0x40

/* Interrupt control. */
void NABU_DisableInterrupts(void);
void NABU_EnableInterrupts(void);

/* AY register access. */
void ayWrite(uint8_t reg, uint8_t val);
uint8_t ayRead(uint8_t reg);

#endif