# Downland_C: Version Alpha

## INTRODUCTION

Downland is a game for the Tandy Color Computer written by Micheal Aichlmayr. It was released in 1983 by Spectral Associates.

Information about Downland can be found here: https://www.cocopedia.com/wiki/index.php/Downland

This project, Downland_C, is a port of the original game to the C programming language. The basis of this port is from the Downland_RE project, found here: https://github.com/pw32x/Downland_RE.

Downland_C is intended to be a faithful reproduction of the game's graphics, physics, and gameplay. While it is not 100% accurate, it should be very close. 

The project is based on the original Downland, version 1.1. This version was a later release to support the Tandy Color Computer 3.

## SUPPORTED PLATFORMS

- Windows

## RUNNING THE GAME

You will need: 
- a copy of the Downland V1.1 rom
- install it in the same folder as the Downland_C exe.

The project loads resources from the Downland rom. If the rom isn't present the game will not run.

Downland_C supports multiple rom names:
- downland.bin
- downland.rom
- Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc


## CONTROLING THE GAME

You will need:
- a keyboard
- (optionally) a gamepad. Xbox Series X/S and PS5 gamepads are supported (as they're the ones I have)

Currently only supports one gamepad. For two players you'll have to share!


## CONTROLS
    
    Keyboard    Arrow keys for Movement
                Left Shift/LeftCtrl/Z for jump
    
    Gamepad     Dpad or Left Analog Stick for Movement
                A (Xbox) or X (PS5) for jump
                
    Escape      Pause/Unpause
    Space       Step Pause
    F1          Change video filter (see below)
    Alt+Enter   Toggle Fullscreen
    Alt+F4      Close

## VIDEO FILTERS

The game has basic support for different graphics looks. 
The F1 key cycles through:
- a basic blue CRT artifact mode
- a basic orange CRT artifact mode
- raw 1bpp mode

## GAMEPLAY NOTES

- the number of drops in a room is 6 for Chambers 0 to 5 and (Chamber Number + 1) for chambers 6 to 9
- the maximum number of active drops in a room is 10
- drops constantly wiggle for 40 frames
- every room has a unique timer. The rooms you're on in will have their timers increase.

Completing the game once will:
- enable all 10 drops for every room
- slightly change the order in which doors are opened by keys

Completing the game three times will:
- tweak the drops' horizontal position such that those that fall to the right of the vine will touch the player


## SCORING SYSTEM

There are exactly five pick ups per room
	The first two are always keys, the last three are treasures
	Treasures are randomly chosen at game start up
	   Key:        200 points
	   Money bag:  300 points
	   Diamond:    400 points

    The scoring system also has a random-based component. The three treasures are randomly chosen at game 
    start. When they're picked up, the player gets the base points, plus the game adds a random amount 
    anywhere from 0 to 127. 
    
    In the original game, the random points depend on an wandering pointer that goes linearly along 
    the rom's address space (c000 - df5a). When those points are needed, the value at the pointer's 
    current location is read and the first 7 bits are returned. Here, it's just a value generated from
    a call to the C rand() function.


## DEBUGGING

    For alpha testing, Downland_C has extra controls:
    
    Keyboard    Tab to toggle debug movement mode
                Tilde Key to return to the title screen
                1 to 0 keys to jump to a room
    
    Gamepad     B (Xbox) or O (PS5) to toggle debug movement mode
    
    Debug movement mode disables collisions with hazards but keeps
    collisions with treasures. 
    




## CONTACT

Send questions, comments, & bugs about Downland_C to pw_32x at: jump@puffweet.com




