// psgdac - plays the sound effects: one frame of AY-3-8910 register writes at a time.  This is psgdac.s of the MSX port
// of Downland_C (Downland_Vibed/platforms/msx/source) for the NABU: only the sound chip's ports differ, and the mixer
// register (7) keeps the direction bits of the chip's two I/O ports as the system left them, because the NABU keeps
// its interrupt mask on I/O port A (register 14) of the same chip.
//
// The original game on the Color Computer makes its sounds by writing values to the machine's DAC and waiting a
// precise number of cycles between the writes.  The AY has no DAC, but a channel with its tone and noise switched off
// in the mixer stays on, and its volume register works as one.  The NABU has no timer or line interrupt to time
// pulses with, and waiting for them with the CPU takes too much of a frame, so the sound chip is only changed once a
// frame: right after the frame starts, the main loop calls
//
//     psg_start_frame();
//
// with psg_ptr pointing at the frame to play.  An effect is a stream of frames made from the original recordings by
// coco2ay.py.  A frame is
//
//     n  (r v) ... (r v)       n register writes: register r (0 to 15) gets the value v
//                              (n = $FF ends the effect)
//     then, once if there is   $02 level lo hi count
//     one                      a square wave of 'count' periods on channel A: the volume register as a DAC at 'level',
//                              waits (hi * 256 + lo) * 26 cycles, 0, waits the same, and again.  The CPU waits.
//     and $00 to end the frame
//
// psg_start_frame plays the frame and leaves the start of the next frame in psg_next.  psg_done is 1 when the effect
// has ended.  The only effect with a square wave is the splat.
//
// The AY is at ports $41 (register) and $40 (data).  Register 14 is left selected, as z88dk's interrupt code expects.

const unsigned char *psg_ptr;          // the frame to play (set by the caller)
const unsigned char *psg_next;         // the frame after it
unsigned char psg_done;
unsigned char psg_mixer_kept;          // bits 7-6 of the mixer register as the system left them (set by the caller)

void psg_start_frame(void) __naked
{
    __asm
    xor     a
    ld      (_psg_done),a
    ld      hl,(_psg_ptr)
    ld      a,(hl)
    inc     hl
    cp      0xFF
    jr      z,psgdac_end_effect
    or      a
    jr      z,psgdac_square_record
    ld      b,a
psgdac_write_loop:
    ld      a,(hl)                  ; the register
    inc     hl
    out     (0x41),a
    cp      7
    ld      a,(hl)                  ; the value (this load does not change the flags)
    inc     hl
    jr      nz,psgdac_plain
    and     0x3F                    ; the mixer: its tone and noise bits only
    ld      c,a
    ld      a,(_psg_mixer_kept)
    or      c
psgdac_plain:
    out     (0x40),a
    djnz    psgdac_write_loop
psgdac_square_record:
    ld      a,(hl)                  ; 2 for a square wave, 0 for the end of the frame
    inc     hl
    or      a
    jr      z,psgdac_frame_end
    ld      d,(hl)                  ; the level
    inc     hl
    ld      c,(hl)                  ; BC = iterations of the wait in half a period
    inc     hl
    ld      b,(hl)
    inc     hl
    ld      e,(hl)                  ; the number of periods
    inc     hl
    push    hl                      ; HL is the wait counter below
    ld      a,8
    out     (0x41),a                ; volume of channel A: stays selected for the writes below
psgdac_square_loop:
    ld      a,d
    out     (0x40),a
    ld      h,b
    ld      l,c
psgdac_square_on:
    dec     hl
    ld      a,h
    or      l
    jr      nz,psgdac_square_on
    xor     a
    out     (0x40),a
    ld      h,b
    ld      l,c
psgdac_square_off:
    dec     hl
    ld      a,h
    or      l
    jr      nz,psgdac_square_off
    dec     e
    jr      nz,psgdac_square_loop
    pop     hl
    jr      psgdac_square_record
psgdac_frame_end:
    ld      (_psg_next),hl
    ld      a,14                    ; leave the interrupt mask register selected
    out     (0x41),a
    ret
psgdac_end_effect:
    ld      a,1
    ld      (_psg_done),a
    ret
    __endasm;
}
