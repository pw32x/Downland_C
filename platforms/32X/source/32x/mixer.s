        .text

! void MixSamples(void *mixer, int16_t *buffer, int32_t cnt, int32_t scale);
! On entry: r4 = mixer pointer
!           r5 = buffer pointer
!           r6 = count (number of stereo 16-bit samples)
!           r7 = scale (global volume - possibly fading, 0 - 64)
        .align  4
        .global _MixSamples
_MixSamples:
        mov.l   r8,@-r15
        mov.l   r9,@-r15
        mov.l   r10,@-r15
        mov.l   r11,@-r15
        mov.l   r12,@-r15
        mov.l   r13,@-r15
        mov.l   r14,@-r15

        mov.l   @r4,r8          /* data pointer */
        mov.l   @(4,r4),r9      /* position */
        mov.l   @(8,r4),r10     /* increment */
        mov.l   @(12,r4),r11    /* length */
        mov.l   @(16,r4),r12    /* loop_length */
        mov.w   @(20,r4),r0     /* volume:pan */

        /* calculate left/right volumes from volume, pan, and scale */
        mov     r0,r13
        shlr8   r13
        extu.b  r13,r13         /* ch_vol */
        mov     r13,r14
        extu.b  r0,r0           /* pan */

        .ifdef  INTENSITY_CROSSFADE
        mov     #0xFF,r1
        extu.b  r1,r1
        sub     r0,r1           /* 255 - pan */
        mulu.w  r0,r0
        sts     macl,r0         /* pan**2 */
        mulu.w  r0,r14
        sts     macl,r0         /* pan**2 * ch_vol */
        mul.l   r0,r7
        sts     macl,r14        /* pan**2 * ch_vol * scale */
        shlr16  r14
!       shlr2   r14
        shlr2   r14             /* right volume = pan**2 * ch_vol * scale / 256 / 64 / 64 */

        mulu.w  r1,r1
        sts     macl,r0         /* (255 - pan)**2 */
        mulu.w  r0,r13
        sts     macl,r0         /* (255 - pan)**2 * ch_vol */
        mul.l   r0,r7
        sts     macl,r13        /* (255 - pan)**2 * ch_vol * scale */
        shlr16  r13
!       shlr2   r13
        shlr2   r13             /* left volume = (255 - pan)**2 * ch_vol * scale / 256 / 64 / 64 */
        .endif

        .ifdef  LINEAR_CROSSFADE
        mov     #0xFF,r1
        extu.b  r1,r1
        sub     r0,r1           /* 255 - pan */
        mulu.w  r0,r14
        sts     macl,r0         /* pan * ch_vol */
        mul.l   r0,r7
        sts     macl,r14        /* pan * ch_vol * scale */
        shlr8   r14
!       shlr2   r14
        shlr2   r14             /* right volume = pan * ch_vol * scale / 64 / 64 */

        mulu.w  r1,r13
        sts     macl,r0         /* (255 - pan) * ch_vol */
        mul.l   r0,r7
        sts     macl,r13        /* (255 - pan) * ch_vol * scale */
        shlr8   r13
!       shlr2   r13
        shlr2   r13             /* left volume = (255 - pan) * ch_vol * scale / 64 / 64 */
        .endif

        .ifdef  EQUAL_DISTANCE_CROSSFADE
        mov     r0,r1
        mov.l   edc_left_ptr,r2
        mov.b   @(r0,r2),r0
        extu.b  r0,r0           /* epan = edc_left[pan] */
        mulu.w  r0,r13
        sts     macl,r0         /* epan * ch_vol */
        mul.l   r0,r7
        sts     macl,r13        /* epan * ch_vol * scale */
        shlr8   r13
!       shlr2   r13
        shlr2   r13             /* left volume = epan * ch_vol * scale / 64 / 64 */

        mov     r1,r0
        mov.l   edc_right_ptr,r2
        mov.b   @(r0,r2),r1
        extu.b  r1,r1           /* epan = edc_right[pan] */
        mulu.w  r1,r14
        sts     macl,r0         /* epan * ch_vol */
        mul.l   r0,r7
        sts     macl,r14        /* epan * ch_vol * scale */
        shlr8   r14
!       shlr2   r14
        shlr2   r14             /* right volume = epan * ch_vol * scale / 64 / 64 */
        .endif

        /* mix r6 stereo samples */
mix_loop:
        /* process one sample */
        mov     r9,r0
        shlr8   r0
        shll2   r0
        shlr8   r0
        mov.b   @(r0,r8),r3
        /* scale sample for left output */
        muls.w  r3,r13
        mov.w   @r5,r1
        sts     macl,r0
        shlr8   r0
        exts.w  r0,r0
        add     r0,r1
        mov.w   r1,@r5
        add     #2,r5
        /* scale sample for right output */
        muls.w  r3,r14
        mov.w   @r5,r1
        sts     macl,r0
        shlr8   r0
        exts.w  r0,r0
        add     r0,r1
        mov.w   r1,@r5
        add     #2,r5

        /* advance position and check for loop */
        add     r10,r9                  /* position += increment */
mix_chk:
        cmp/hs  r11,r9
        bt      mix_wrap                /* position >= length */
mix_next:
        /* next sample */
        dt      r6
        bf      mix_loop
        bra     mix_exit
        mov.l   r9,@(4,r4)              /* update position field */

mix_wrap:
        /* check if loop sample */
        mov     r12,r0
        cmp/eq  #0,r0
        bf/s    mix_chk                 /* loop sample */
        sub     r12,r9                  /* position -= loop_length */
        /* sample done playing */
        mov.l   r12,@r4                 /* clear data pointer field */

mix_exit:
        mov.l   @r15+,r14
        mov.l   @r15+,r13
        mov.l   @r15+,r12
        mov.l   @r15+,r11
        mov.l   @r15+,r10
        mov.l   @r15+,r9
        mov.l   @r15+,r8
        rts
        nop

! void LockMixer(int16_t id)
! Entry: r4 = id

        .global _LockMixer
_LockMixer:
        exts.w  r4,r4
        mov.l   mixer_state,r1
0:
        mov.w   @r1,r0
        cmp/eq  #1,r0                   /* loop until unlocked */
        bf      0b

        mov.w   r4,@r1
        mov.w   @r1,r0
        cmp/eq  r4,r0
        bf      0b                      /* race condition - we lost */

        rts
        nop

! void UnlockMixer(void)

        .global _UnlockMixer
_UnlockMixer:
        mov     #1,r0
        mov.l   mixer_state,r1
        rts
        mov.w   r0,@r1


        .align  2

        .equ    MARS_SYS_COMM6, 0x20004026

mixer_state:
        .long   MARS_SYS_COMM6

        .ifdef  EQUAL_DISTANCE_CROSSFADE
edc_left_ptr:
        .long   edc_left
edc_right_ptr:
        .long   edc_right
        .include "edc_table.i"
        .endif

        .align  2

