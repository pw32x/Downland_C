# Downland_C: Version Alpha

## INTRODUCTION

**Downland** is a game for the Tandy Color Computer written by **Micheal Aichlmayr**. It was released in 1983 by Spectral Associates.

Information about Downland can be found here: https://www.cocopedia.com/wiki/index.php/Downland

This project, **Downland_C**, is a port of the original game to the C programming language. The basis of this port is from the **Downland_RE** project, found here: https://github.com/pw32x/Downland_RE.

Downland_C is intended to be a faithful reproduction of the game's graphics, physics, and gameplay. While it is not 100% accurate, it should be very close. 

The project is based on the original Downland, version 1.1. This version was a later release to support the Tandy Color Computer 3.

## SUPPORTED PLATFORMS

- Windows
- Dreamcast (see platforms\cd\README.md)

## RUNNING THE GAME

You will need: 
- A copy of the Downland V1.1 rom.
- Install it in the same folder as the Downland_C exe.

The project loads resources from the Downland rom. If the rom isn't present the game will not run.

Downland_C supports multiple rom names:
- downland.bin
- downland.rom
- Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc


## CONTROLING THE GAME

You will need:
- A keyboard
- (optionally) A gamepad. Xbox Series X/S and PS5 gamepads are supported (as they're the ones I have)

Currently only supports one gamepad. For two players you'll have to share!


## CONTROLS
    
    Keyboard    Arrow keys for Movement
                Left Shift/LeftCtrl/Z for jump
    
    Gamepad     Dpad or Left Analog Stick for Movement
                A (Xbox) or X (PS5) for jump
                
    Escape      Pause/Unpause
    Space       Step Pause
    F1          Cycle through video filters (see below)
    Alt+Enter   Toggle Fullscreen
    Alt+F4      Close the application

## VIDEO FILTERS

The game has basic support for different graphics looks. 
The F1 key cycles through:
- A basic blue CRT artifact mode.
- A basic orange CRT artifact mode.
- A raw 1bpp mode.

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

## DEBUGGING

For alpha testing, Downland_C has extra controls:
    
    Keyboard    Tab to toggle debug movement mode
                Tilde Key to return to the title screen
                1 to 0 keys to jump to a room
    
    Gamepad     B (Xbox) or O (PS5) to toggle debug movement mode
    
Debug movement mode disables collisions with hazards but keeps collisions with pickups. 

## CONTACT

Send questions, comments, & bugs about Downland_C to pw32x at jump@puffweet.com




