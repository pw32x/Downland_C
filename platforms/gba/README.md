# Downland_C: GameBoy Advance

## INTRODUCTION

This is the Nintendo GameBoy Advance port of Downland_C.

## BUILDING THE GAME

You will need: 

- This repository
- An install of the devkitPro (https://github.com/devkitPro/installer/releases), v3.0.3
- A copy of the Downland V1.1 rom located in the gamedata folder

Instructions:
    
- Install the devkitPro. 
- Run the build.bat in the build folder or use the provided Visual Studio solution.
- The built downland.gba rom will be in the out folder.

## RUNNING THE GAME

You will need: 
- a GameBoy Advance with flashcart, or
- or, an emulator like mGBA
- a copy of the generated downland.gba rom

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

## CONTACT

Send questions, comments, & bugs about Downland_C to pw32x at jump@puffweet.com
                