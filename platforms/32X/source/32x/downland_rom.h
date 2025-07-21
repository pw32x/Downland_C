#ifndef DOWNLAND_ROM_INCLUDE_H
#define DOWNLAND_ROM_INCLUDE_H

// assumes there is a Downland V1.1 rom in the gamedata folder
// which the Makefile will create a downland.o file in the /out
// folder.

extern const unsigned char binary_gamedata_downland_rom_start[];
extern const unsigned char binary_gamedata_downland_rom_end[];
extern const unsigned int  binary_gamedata_downland_rom_size;

#endif // DOWNLAND_ROM_INCLUDE_H