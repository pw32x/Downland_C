# to build you'll need to define the NABU_COMPILER environment variable
# to point to z88dk's root folder (ie C:\development\NABU\z88dk)

# you can change the PROGNAME by passing it as a parameter to make: "make -f build/Makefile PROGNAME=my_game".
# the resulting rom will be Downland.nabu. Since a real NABU (and the Marduk emulator) boots 
# segment 000001 we later create a copy called 0000001.nabu.
PROGNAME?=Downland

# convert slashes from Windows-only to universal
NABU_COMPILER_DIR := $(subst \,/,$(NABU_COMPILER))

CC=$(NABU_COMPILER_DIR)/bin/zcc
export ZCCCFG := $(NABU_COMPILER_DIR)/lib/config
TARGET=+nabu -compiler=sdcc #use sdcc because the default one suuuuuuuucks

SOURCE_DIR = source
OUTPUT_DIR = out
SOURCE_OUTPUT_DIR = $(OUTPUT_DIR)/$(SOURCE_DIR)

# common Downland game code for 8-bit platforms
GAME_DIR = ../../game_8bit
GAME_OUTPUT_DIR = $(OUTPUT_DIR)/game

GENERATED_DIR = generated
GENERATED_OUTPUT_DIR = $(OUTPUT_DIR)/generated
GENERATED_DIRS = $(GENERATED_DIR) $(GENERATED_DIR)/bank2 $(GENERATED_DIR)/bank3 $(GENERATED_DIR)/bank4 $(GENERATED_DIR)/bank5 $(GENERATED_DIR)/bank6 $(GENERATED_DIR)/bank7 $(GENERATED_DIR)/bank8

# c flags. Include ENABLE_CLEAN_BACKGROUND_DRAW because the NABU has enough ram to have
# a clean background buffer to draw directy into. 
CFLAGS=$(TARGET) -vn -SO3 -I$(SOURCE_DIR) -I$(GENERATED_DIR) -I$(GAME_DIR) -DENABLE_CLEAN_BACKGROUND_DRAW

# apparently variables in ram are included to the rom as a bss section.
# add the pragma to move them to a separate file, reducing the final rom size.
LDFLAGS=$(TARGET) -vn -m -pragma-define:__crt_org_bss=0xB800

# get project, game, and generated source files
CS			= $(shell find $(SOURCE_DIR) -name '*.c')
HEADERS		= $(shell find $(SOURCE_DIR) -name '*.h')
GAME_CS		= $(shell find $(GAME_DIR) -name '*.c')
GAME_HEADERS	= $(shell find $(GAME_DIR) -name '*.h')
GENERATED_CS	= $(foreach d,$(GENERATED_DIRS),$(wildcard $(d)/*.c))
GENERATED_HEADERS = $(shell find $(GENERATED_DIR) -name '*.h')

ALL_HEADERS	= $(HEADERS) $(GAME_HEADERS) $(GENERATED_HEADERS)

OBJS	= $(patsubst $(SOURCE_DIR)/%.c,$(SOURCE_OUTPUT_DIR)/%.o,$(CS))
OBJS	+= $(patsubst $(GAME_DIR)/%.c,$(GAME_OUTPUT_DIR)/%.o,$(GAME_CS))
OBJS	+= $(patsubst $(GENERATED_DIR)/%.c,$(GENERATED_OUTPUT_DIR)/%.o,$(GENERATED_CS))

all: $(OUTPUT_DIR)/$(PROGNAME).nabu

$(SOURCE_OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.c $(ALL_HEADERS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_OUTPUT_DIR)/%.o: $(GAME_DIR)/%.c $(ALL_HEADERS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(GENERATED_OUTPUT_DIR)/%.o: $(GENERATED_DIR)/%.c $(ALL_HEADERS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# link, and create a copy of the rom as 000001.nabu
$(OUTPUT_DIR)/$(PROGNAME).nabu: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $(OUTPUT_DIR)/$(PROGNAME) -create-app
	cp $@ $(OUTPUT_DIR)/000001.nabu

clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: all clean
