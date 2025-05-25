# KallistiOS ##version##
#
# video/screenshot/Makefile
# Copyright (C) 2024 Andy Barajas
#

TARGET = out/main.elf
BIN = out/main.bin
OBJS = out/main.o
CDROOT = out/cd_root
SCRAMBLED = $(CDROOT)/1ST_READ.BIN
IP_BIN = out/IP.BIN
CDI = out/main.cdi

all: rm-elf $(SCRAMBLED) $(CDI)

include $(KOS_BASE)/Makefile.rules

clean: rm-elf
	-rm -f $(OBJS)
	-rm -f $(BIN)
	-rm -f $(SCRAMBLED)
	-rm -f $(IP_BIN)
	-rm -f $(CDI)
	-rm -f screenshot*.ppm

rm-elf:
	-rm -f $(TARGET)

out:
	mkdir -p out
	
$(CDROOT): | out
	mkdir -p $(CDROOT)	

$(TARGET): out $(OBJS) 
	kos-cc -o $(TARGET) $(OBJS)

# Compile source files to out/
out/%.o: %.c | out
	kos-cc -c $< -o $@	
	
$(BIN): $(TARGET)
	elf2bin $(TARGET) $(BIN)	

$(SCRAMBLED): $(BIN) $(CDROOT)
	scramble $(BIN) $(SCRAMBLED)
	
$(IP_BIN): | out
	makeip -v -f $(IP_BIN)	
	
$(CDI): $(SCRAMBLED) $(IP_BIN)
	makedisc $(CDI) $(CDROOT) $(IP_BIN) main	

# For Mac, Linux, Ubuntu sudo is required with the -c "." command to save data to the PC
#    sudo $(KOS_LOADER) $(TARGET) -c "."
run: $(TARGET)
	$(KOS_LOADER) $(TARGET) -c "."

dist: $(BIN)
	-rm -f $(OBJS)
	$(KOS_STRIP) $(TARGET)
