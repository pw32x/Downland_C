# KallistiOS ##version##
#
# video/screenshot/Makefile
# Copyright (C) 2024 Andy Barajas
#

# Paths relative to this Makefile (inside build/)
SRC_ROOT = ..
GAME_ROOT = $(SRC_ROOT)/../../game
ROOMS_ROOT = $(GAME_ROOT)/rooms
OUT_DIR = $(SRC_ROOT)/out
CDROOT = $(OUT_DIR)/cd_root

TARGET = $(OUT_DIR)/main.elf
BIN = $(OUT_DIR)/main.bin
SCRAMBLED = $(CDROOT)/1ST_READ.BIN
IP_BIN = $(OUT_DIR)/IP.BIN
CDI = $(OUT_DIR)/main.cdi


# Compiler flags
CFLAGS = -Werror

# Source files
LOCAL_SRCS = $(wildcard $(SRC_ROOT)/source/*.c)
GAME_SRCS = $(wildcard $(GAME_ROOT)/*.c)
ROOM_SRCS = $(wildcard $(ROOMS_ROOT)/*.c)

# Object file output mapping
LOCAL_OBJS = $(patsubst $(SRC_ROOT)/source/%.c,$(OUT_DIR)/source/%.o,$(LOCAL_SRCS))
GAME_OBJS = $(patsubst $(GAME_ROOT)/%.c,$(OUT_DIR)/game/%.o,$(GAME_SRCS))
ROOM_OBJS = $(patsubst $(ROOMS_ROOT)/%.c,$(OUT_DIR)/game/rooms/%.o,$(ROOM_SRCS))


OBJS = $(LOCAL_OBJS) $(GAME_OBJS) $(ROOM_OBJS)

# Include paths
INCLUDES = -I$(SRC_ROOT)/source -I$(GAME_ROOT) -I$(ROOMS_ROOT)

all: rm-elf prepare_dirs $(CDI)

include $(KOS_BASE)/Makefile.rules

clean: rm-elf
	-rm -f $(OBJS)
	-rm -f $(BIN)
	-rm -f $(SCRAMBLED)
	-rm -f $(IP_BIN)
	-rm -f $(CDI)
	-rm -f $(SRC_ROOT)/screenshot*.ppm

rm-elf:
	-rm -f $(TARGET)

# Ensure directory structure exists
prepare_dirs: $(OUT_DIR) $(OUT_DIR)/source $(OUT_DIR)/game $(OUT_DIR)/game/rooms $(CDROOT)

$(OUT_DIR):
	mkdir -p $@

$(OUT_DIR)/source:
	mkdir -p $@

$(OUT_DIR)/game:
	mkdir -p $@

$(OUT_DIR)/game/rooms:
	mkdir -p $@

$(CDROOT):
	mkdir -p $@

# Compile rules
$(OUT_DIR)/source/%.o: $(SRC_ROOT)/source/%.c | $(OUT_DIR)/source
	kos-cc $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUT_DIR)/game/%.o: $(GAME_ROOT)/%.c | $(OUT_DIR)/game
	kos-cc $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUT_DIR)/game/rooms/%.o: $(ROOMS_ROOT)/%.c | $(OUT_DIR)/game/rooms
	kos-cc $(CFLAGS) $(INCLUDES) -c $< -o $@

# Linking
$(TARGET): $(OBJS)
	kos-cc $(CFLAGS) $(INCLUDES) -o $@ $(OBJS)

# Convert ELF ? BIN
$(BIN): $(TARGET)
	elf2bin $< $@

# Scramble to Dreamcast-ready BIN
$(SCRAMBLED): $(BIN)
	scramble $< $@

# IP.BIN creation
$(IP_BIN): | $(OUT_DIR)
	makeip -v -f $@

# CDI image creation
$(CDI): $(SCRAMBLED) $(IP_BIN)
	makedisc $@ $(CDROOT) $(IP_BIN) main

# Run ELF via loader
run: $(TARGET)
	$(KOS_LOADER) $(TARGET) -c "."

# Distribution
dist: $(CDI)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)