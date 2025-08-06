        .text

        .align  4
file1:      .incbin "sounds/jump_s8_16000.raw"
fileEnd1:
        .align  4
file2:      .incbin "sounds/land_s8_16000.raw"
fileEnd2:
        .align  4
file3:      .incbin "sounds/transition_s8_16000.raw"
fileEnd3:
        .align  4
file4:      .incbin "sounds/splat_s8_16000.raw"
fileEnd4:
        .align  4
file5:      .incbin "sounds/pickup_s8_16000.raw"
fileEnd5:
        .align  4
file6:      .incbin "sounds/run_s8_16000.raw"
fileEnd6:
        .align  4
file7:      .incbin "sounds/climb_up_s8_16000.raw"
fileEnd7:
        .align  4
file8:      .incbin "sounds/climb_down_s8_16000.raw"
fileEnd8:


        .align  4
        .global _soundFiles
_soundFiles:
        .long file1, fileEnd1 - file1, 16000
        .long file2, fileEnd2 - file2, 16000
        .long file3, fileEnd3 - file3, 16000
        .long file4, fileEnd4 - file4, 16000
        .long file5, fileEnd5 - file5, 16000
        .long file6, fileEnd6 - file6, 16000
        .long file7, fileEnd7 - file7, 16000
        .long file8, fileEnd8 - file8, 16000
