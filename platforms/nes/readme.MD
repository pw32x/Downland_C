# Downland_C: NES

## INTRODUCTION

This is the NES port of Downland_C.

## BUILDING THE GAME

You will need: 

- This repository
- An install of llvm-mos sdk https://github.com/llvm-mos/llvm-mos-sdk
- A copy of the Downland V1.1 rom placed in the base folder as downland.rom

Instructions:


Building

- download and install llvm-mos sdk
- update LLVM_MOS_SDK in the Makefile to your devkitSMS installation
- build and run the DLExporter solution which will generate assets in the \generated folder
- run build.bat in the build folder or use the provided Visual Studio solution.
- the Downland.nes file will be in the out folder 
   

## RUNNING THE GAME

Since I have no NES I've only been able to test in emulator. 

Emulator
- an emulator (there are very many. I used Mesen https://www.mesen.ca/)
- the built downland.nes as a file 
- load the rom in the emulator and enjoy

## CONTROLS

    The game supports two players. 
    
    Gamepad     Joypad for Movement
                Action buttons for Jump
               
## GAMEPLAY NOTES

- The number of drops in a room is 6 for Chambers 0 to 5 and (Chamber Number + 1) for chambers 6 to 9.
- The maximum number of active drops in a room is 10.
- Drops constantly wiggle for 40 frames.
- Every room has a unique timer. The rooms you're on in will have their timers increase.

Completing the game once will:
- Enable all 10 drops for every room.
- Change the order in which doors are opened by keys.

Completing the game three times will:
- Tweak the drops' horizontal position such that those that fall to the right of the vine will touch the player while climbing.


## SCORING SYSTEM

There are exactly five pick ups per room. 
The first two are keys and the last three are treasures randomly chosen at game start up.

The pickups are:
- Key:        200 points
- Money bag:  300 points
- Diamond:    400 points

The scoring system also has a random-based component. The three treasures are randomly chosen at game . When they're picked up, the player gets the base points, plus the game adds a random amount from 0 to 127. 

## CONTACT

Send questions, comments, & bugs about Downland_C to pw32x at jump@puffweet.com
                
