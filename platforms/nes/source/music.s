.section .rodata.looping_effects_data,"a",@progbits
.globl looping_effects_data
looping_effects_data:
  .include "LoopingEffects.s"

.section .rodata.sound_effects_data,"a",@progbits
.globl sound_effects_data
sound_effects_data:
  .include "SoundEffects.s"
