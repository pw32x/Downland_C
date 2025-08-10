# Downland_C: 3DO

## INTRODUCTION

This is the 3DO port of Downland_C.

## BUILDING THE GAME

You will need: 

- This repository
- An install of the 3DO devkit (https://github.com/trapexit/3do-devkit)
- A copy of the Downland V1.1 rom placed in the cd_contents folder

Instructions:
  
   
    
- Get the 3DO Devkit from GitHub.
- Ensure that TDO_DEVKIT_PATH is set to the root of the 3DO devkit folder (ie c:\3do-devkit)
- Run the build.bat in the build folder or use the provided Visual Studio solution.
- To build the game to run on the Playdate device, run the create_build_***_arm_project.bat file for debug or release.
- The built downland.iso will be in the out/iso folder.

## RUNNING THE GAME

You will need: 
- a 3DO console
- or, an emulator like 4DO, Pheonix, or Opera
- the built downland.iso as a file or burned to a disk

## CONTROLS

    The game supports two players. 
    
    Gamepad     Dpad for Movement
                A for Jump
                B for debug movement mode (when enabled)
                Start to Pause
                
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

## THANKS

Thanks to Archive3DO and Retro Comparisons for testing on hardware!

## CONTACT

Send questions, comments, & bugs about Downland_C to pw32x at jump@puffweet.com
                