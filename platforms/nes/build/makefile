# =========================================
# NES Hello Makefile (LLVM-MOS)
# =========================================

PROJECT := hello

TARGET  := out/$(PROJECT).nes

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
           -Wl,-Map,out/$(PROJECT).map \
           -L$(LLVM_MOS_LIBDIR)

# ---------------------------
# Source files
# ---------------------------
SRC_C   := source/hello.c
SRC_ASM := source/chr-rom.s
OBJ_C   := out/hello.o
OBJ_ASM := out/chr-rom.o

# The CHR dependency
CHR_FILE := res/downlandTileset.chr
# ---------------------------
# PHONY targets
# ---------------------------
.PHONY: all clean

# ---------------------------
# Default target
# ---------------------------
all: $(TARGET)

# ---------------------------
# Link ROM
# ---------------------------
$(TARGET): $(OBJ_C) $(OBJ_ASM)
	$(CC) $(CFLAGS) $(OBJ_C) $(OBJ_ASM) $(LDFLAGS) -lneslib -o $@

# ---------------------------
# Compile C file
# ---------------------------
out/%.o: source/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------
# Assemble ASM file
# ---------------------------
# Make chr-rom.s depend on downlandTileset.chr
out/%.o: source/%.s $(CHR_FILE)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------
# Clean
# ---------------------------
clean:
	rm -f $(OBJ_C) $(OBJ_ASM) $(TARGET) out/$(PROJECT).map out/$(PROJECT).nes.elf out/$(PROJECT).mlb
