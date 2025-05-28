# KallistiOS ##version##

PROJECT = downland

# Paths relative to this Makefile (inside build/)
SRC_ROOT = ..
GAME_ROOT = $(SRC_ROOT)/../../game
ROOMS_ROOT = $(GAME_ROOT)/rooms
RESOURCELOADERS_ROOT = $(SRC_ROOT)/../../resource_loaders
OUT_DIR = $(SRC_ROOT)/out
CDROOT = $(OUT_DIR)/cd_root
ROMDISK_ROOT = $(SRC_ROOT)/romdisk

TARGET = $(OUT_DIR)/$(PROJECT).elf
BIN = $(OUT_DIR)/$(PROJECT).bin
SCRAMBLED = $(CDROOT)/1ST_READ.BIN
IP_BIN = $(OUT_DIR)/IP.BIN
CDI = $(OUT_DIR)/$(PROJECT).cdi



# Compiler flags
CFLAGS = -Werror

# Source files
LOCAL_SRCS = $(wildcard $(SRC_ROOT)/source/*.c)
GAME_SRCS = $(wildcard $(GAME_ROOT)/*.c)
ROOM_SRCS = $(wildcard $(ROOMS_ROOT)/*.c)
RESOURCELOADERS_SRCS = $(wildcard $(RESOURCELOADERS_ROOT)/*.c)

# Object file output mapping
LOCAL_OBJS = $(patsubst $(SRC_ROOT)/source/%.c,$(OUT_DIR)/source/%.o,$(LOCAL_SRCS))
GAME_OBJS = $(patsubst $(GAME_ROOT)/%.c,$(OUT_DIR)/game/%.o,$(GAME_SRCS))
ROOM_OBJS = $(patsubst $(ROOMS_ROOT)/%.c,$(OUT_DIR)/game/rooms/%.o,$(ROOM_SRCS))
RESOURCELOADERS_OBJS = $(patsubst $(RESOURCELOADERS_ROOT)/%.c,$(OUT_DIR)/resource_loaders/%.o,$(RESOURCELOADERS_SRCS))
ROMDISK_OBJ = $(OUT_DIR)/romdisk.o

OBJS = $(LOCAL_OBJS) $(GAME_OBJS) $(ROOM_OBJS) $(ROMDISK_OBJ) $(RESOURCELOADERS_OBJS)

# Include paths
INCLUDES = -I$(SRC_ROOT)/source -I$(GAME_ROOT) -I$(ROOMS_ROOT) -I$(RESOURCELOADERS_ROOT)



all: rm-elf prepare_dirs $(CDI)

include $(KOS_BASE)/Makefile.rules

#custom romdisk and output location
$(OUT_DIR)/romdisk.img:
	$(KOS_GENROMFS) -f $(OUT_DIR)/romdisk.img -d $(ROMDISK_ROOT) -v -x .keepme -x .DS_Store -x Thumbs.db

$(OUT_DIR)/romdisk.o: $(OUT_DIR)/romdisk.img
	$(KOS_BASE)/utils/bin2c/bin2c $(OUT_DIR)/romdisk.img $(OUT_DIR)/romdisk_tmp.c romdisk
	$(KOS_CC) $(KOS_CFLAGS) -o $(OUT_DIR)/romdisk_tmp.o -c $(OUT_DIR)/romdisk_tmp.c
	$(KOS_CC) -o $(OUT_DIR)/romdisk.o -r $(OUT_DIR)/romdisk_tmp.o $(KOS_LIB_PATHS) -Wl,--whole-archive -lromdiskbase
	rm $(OUT_DIR)/romdisk_tmp.c $(OUT_DIR)/romdisk_tmp.o

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
prepare_dirs: $(OUT_DIR) $(OUT_DIR)/source $(OUT_DIR)/game $(OUT_DIR)/game/rooms $(OUT_DIR)/resource_loaders $(CDROOT)

$(OUT_DIR):
	mkdir -p $@

$(OUT_DIR)/source:
	mkdir -p $@

$(OUT_DIR)/game:
	mkdir -p $@
	
$(OUT_DIR)/resource_loaders:
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
	
$(OUT_DIR)/resource_loaders/%.o: $(RESOURCELOADERS_ROOT)/%.c | $(OUT_DIR)/resource_loaders
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
	makedisc $@ $(CDROOT) $(IP_BIN) $(PROJECT)

# Run ELF via loader
run: $(TARGET)
	$(KOS_LOADER) $(TARGET) -c "."

# Distribution
dist: $(CDI)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)