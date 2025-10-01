mkdir psg
java -jar PSGTool\PSGTool.jar explosion.vgm psg\explosion.psg 23
java -jar PSGTool\PSGTool.jar run.vgm psg\run.psg 23
java -jar PSGTool\PSGTool.jar land.vgm psg\land.psg 23
java -jar PSGTool\PSGTool.jar pickup.vgm psg\pickup.psg 2
java -jar PSGTool\PSGTool.jar jump.vgm psg\jump.psg 2


del ..\generated\sounds.c
del ..\generated\sounds.h 

pushd ..\generated
..\..\..\..\..\..\SMSDev\devkitSMS-master\assets2banks\Windows\assets2banks.exe ..\sounds\psg
popd

rename ..\generated\bank2.h sounds.h
rename ..\generated\bank2.c sounds.c