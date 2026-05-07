# Downland_C: ESP32 Cheap Yellow Display (ESP32-2432S028)

## INTRODUCTION

This is the Cheap Yellow Display port of Downland_C.


## NOTES ABOUT THIS VERSION

This version's performance is not great. This was pretty much an experiment. It's running at about less than half the frame rate because I'm blitting the whole screen to the device. It would need a 32X-style dirty rectangle drawing scheme.

I couldn't get the project to use the gameplay code in \game, so I made a copy into this folder.

## BUILDING THE GAME

You will need: 

- This repository
- An install of Arduino IDE https://www.arduino.cc/en/software/#ide or Visual Micro https://www.visualmicro.com/
- A Cheap Yellow Display. I've used an ESP32-2432S028. Maybe others will work. Who knows!

Instructions:


Building

- Download and install Arduino IDE or Visual Micro
- open the esp32_cyd.ino file
- Install the libraries esp32_cyd.ino needs 
- This User_Setup.h for the TFT_eSPI library: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/DisplayConfig/User_Setup.h

## RUNNING THE GAME

To run on hardware you'll need: 
- A Cheap Yellow Display. I've used an ESP32-2432S028 from AliExpress.
- A copy of the Downland V1.1 rom placed in the base folder of the SD card. (downland.bin, downland.rom, or "Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc")
- build and deploy the game to the board

## CONTROLS

    The game supports two players. 
    
    The controls are VERY rudimentary.
    
    Title screen:
    - Tap the left edge of the screen for a 1 player game. 
    - Tap the right edge for a 2 player game.
    
    Gameplay:
    - Tap the left edge to jump towards the left
    - Tap the right edge to jump towards the right
    - Touch the top center of the screen to go up
    - Touch the bottom center of the screen to go down
    - Touch from the center to the left edge to go left
    - Touch from the center to the right edge to go right
 
               
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

Send questions, comments, & bugs about Downland_C to pw at jump@puffweet.com
                

