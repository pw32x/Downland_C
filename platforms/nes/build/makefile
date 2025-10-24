# =========================================
# NES Downland Makefile (LLVM-MOS)
# =========================================

PROJECT := Downland

# ---------------------------
# LLVM-MOS SDK / NES Platform
# ---------------------------
LLVM_MOS_SDK    := ../../../../../NesDev/llvm-mos-windows/llvm-mos
LLVM_MOS_BIN    := $(LLVM_MOS_SDK)/bin
LLVM_MOS_LIBDIR := $(LLVM_MOS_SDK)/mos-platform/nes/lib
LLVM_MOS_INCDIR := $(LLVM_MOS_SDK)/mos-platform/nes/include

CC  := $(LLVM_MOS_BIN)/clang

# ---------------------------
# Compile flags
# ---------------------------
CFLAGS := -target mos-nes-nrom \
          -Os -g -gdwarf-4 -Wall -Wextra \
          -std=c23 -I.

# ---------------------------
# Linker flags
# ---------------------------
LDFLAGS := -g -gdwarf-4 -Wall -Wextra -Werror \
           -Wl,-Map,$(OUTPUT_DIR)/$(PROJECT).map \
           -L$(LLVM_MOS_LIBDIR)

# ---------------------------
# Source files
# ---------------------------

SOURCE_DIR = source
GENERATED_DIR = generated

OUTPUT_DIR = out

SOURCE_OUTPUT_DIR = $(OUTPUT_DIR)/$(SOURCE_DIR)
GENERATED_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_DIR)

TARGET  := $(OUTPUT_DIR)/$(PROJECT).nes

CS				= $(wildcard $(SOURCE_DIR)/*.c)
CS_ASM          = $(wildcard $(SOURCE_DIR)/*.s)
GENERATED_CS	= $(wildcard $(GENERATED_DIR)/*.c)

SOURCE_OBJS				= $(patsubst $(SOURCE_DIR)/%.c,$(SOURCE_OUTPUT_DIR)/%.o,$(CS))
SOURCE_ASM_OBJS			= $(patsubst $(SOURCE_DIR)/%.s,$(SOURCE_OUTPUT_DIR)/%.o,$(CS_ASM))
GENERATED_OBJS			= $(patsubst $(GENERATED_DIR)/%.c,$(GENERATED_OUTPUT_DIR)/%.o,$(GENERATED_CS))

# The CHR dependency
CHR_FILE := $(GENERATED_DIR)/downlandTileset.chr
# ---------------------------
# PHONY targets
# ---------------------------
.PHONY: directories all clean

# ---------------------------
# Default target
# ---------------------------
all: directories $(TARGET)

directories:
	mkdir -p $(SOURCE_OUTPUT_DIR)
	mkdir -p $(GENERATED_OUTPUT_DIR)

# ---------------------------
# Link ROM
# ---------------------------
$(TARGET): $(SOURCE_OBJS) $(SOURCE_ASM_OBJS)
	$(CC) $(CFLAGS) $(SOURCE_OBJS) $(SOURCE_ASM_OBJS) $(LDFLAGS) -lneslib -o $@

# ---------------------------
# Compile C file
# ---------------------------
$(SOURCE_OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------
# Assemble ASM file
# ---------------------------
# Make chr-rom.s depend on downlandTileset.chr
$(SOURCE_OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.s $(CHR_FILE)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------
# Clean
# ---------------------------
clean:
	rm -f $(SOURCE_OBJS) $(SOURCE_ASM_OBJS) $(TARGET) $(OUTPUT_DIR)/$(PROJECT).map $(OUTPUT_DIR)/$(PROJECT).nes.elf $(OUTPUT_DIR)/$(PROJECT).mlb
