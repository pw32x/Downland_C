
!-----------------------------------------------------------------------
! Sound Manager DMA 1 TE INT handler
!-----------------------------------------------------------------------

        .global soundmanager_dma_handler_vector

soundmanager_dma_handler_vector:
        ! save registers
        sts.l   pr,@-r15
        mov.l   r0,@-r15
        mov.l   r1,@-r15
        mov.l   r2,@-r15
        mov.l   r3,@-r15
        mov.l   r4,@-r15
        mov.l   r5,@-r15
        mov.l   r6,@-r15
        mov.l   r7,@-r15

        mov.l   soundmanager_dma_handler,r0
        jsr     @r0
        nop

        ! restore registers
        mov.l   @r15+,r7
        mov.l   @r15+,r6
        mov.l   @r15+,r5
        mov.l   @r15+,r4
        mov.l   @r15+,r3
        mov.l   @r15+,r2
        mov.l   @r15+,r1
        mov.l   @r15+,r0
        lds.l   @r15+,pr
        rte
        nop

        .align  2
soundmanager_dma_handler:
        .long   _SoundManager_updateDma

