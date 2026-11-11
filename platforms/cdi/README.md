# Downland_C: Philips CD-i

## INTRODUCTION

This is the Philips CD-i port of Downland_C.

## BUILDING THE GAME

You will need: 

- This repository
- A copy of the CD-i SDK (https://github.com/TwBurn/cdi-sdk)
- DOSBox (https://www.dosbox.com/), version 0.74-3
- A copy of the Downland V1.1 rom placed in the gamedata folder as downland.rom

Instructions:

- Set three environment variables

    CDI_SDK_PATH        the path to the CD-i SDK
    CDI_DOSBOX_PATH     the path to the DOSBox folder
    CDIEMU_PATH         the path to the CD-i Emulator executable (used by run.bat)

    examples: 

        CDI_SDK_PATH = C:\development\CDI\cdi-sdk-master
        CDI_DOSBOX_PATH = C:\development\CDI\DOSBox-0.74-3
        CDIEMU_PATH = "C:\development\CDI\cdiemu-0.5.3-beta10\wcdiemu-v053b10.exe"

- Run the build.bat in the build folder or use the provided Visual Studio solution.
- The disc image is mastered with vcdmastr.exe, a 16-bit DOS app, so the build runs it through DOSBox.
- The built DOWNLAND.CDI image will be in the out\disk folder.

## RUNNING THE GAME

You will need: 
- a Philips CD-i, or
- an emulator like CD-i Emulator or MAME
- a copy of the generated DOWNLAND.CDI

run.bat in the build folder launches the game in CD-i Emulator.

## CONTROLS

    The game supports two players. 
    
    Thumbpad    Dpad or mouse for Movement
                Any button for Jump
                
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
