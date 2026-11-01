/*
 * debug_printf support: the message buffer and the routine that tells the emulator.  See debug.h.
 */

#include "debug.h"

char debug_buf[DEBUG_BUFSIZE];

/* __z88dk_fastcall: the argument arrives in HL, which is exactly what the emulator reads.
   __naked: no prologue or epilogue, so HL is still the argument when the trap runs.
   Do not add statements before the __asm block: they would change HL. */
void debug_puts(const char *s) __z88dk_fastcall __naked
{
    __asm
        ld      d,d                     ; the trap: a no-op on real hardware
        jr      debug_trap_end     ; jump over the data bytes below
        defw    0x6464                  ; signature
        defw    0x0100                  ; selector: print the NUL-terminated string at HL
    debug_trap_end:
        ret
    __endasm;
}
