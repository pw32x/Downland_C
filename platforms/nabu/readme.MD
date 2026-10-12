# Downland_C: NABU PC

## INTRODUCTION

This is the NABU PC port of Downland_C. (https://en.wikipedia.org/wiki/NABU_Network)

## BUILDING THE GAME

You will need:

- This repository
- The z88dk compiler https://github.com/z88dk/z88dk (tested with version 2.3)
- A copy of the Downland V1.1 rom placed in the base folder


Instructions:

- set the NABU_COMPILER environment variable (ie C:\development\NABU\z88dk)
- set the NABU_EMULATORS envionrment variable (ie C:\development\NABU\emulators)
- set the NABU_ADAPTERS environment variable (C:\development\NABU\adapters)
- build and run the DLExporter solution to generate the game resources in the generated folder.
- run the convertToC.bat file to generate sprite resources in the generated folder
- run build\build.bat or use the provided Visual Studio solution
- Downland.nabu will be in the out folder

## RUNNING THE GAME

On hardware, you'll obviously need a NABU computer. 

Otherwise,
- an emulator like Marduk (https://github.com/buricco/marduk) or mame (https://github.com/mamedev/mame) 

## CONTROLS

    The game supports two players.

    Joystick    Joystick 1 or 2 for Movement
                Fire for Jump

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

The scoring system also has a random-based component. The three treasures are randomly chosen at game start up. When they're picked up, the player gets the base points, plus the game adds a random amount from 0 to 127.

## CONTACT

Send questions, comments, & bugs about Downland_C to pw32x at jump@puffweet.com
