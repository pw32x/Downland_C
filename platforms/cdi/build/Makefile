NAME    = Downland
OUTPUT  = out
GAMEDATA = gamedata
SRC      = src
DRAW_ERASE = $(SRC)/draw_erase
CDI_HW = $(SRC)/cdi_hw
GAME     = ../../game
ROOMS    = $(GAME)/rooms
DISK     = $(OUTPUT)/disk

# tool locations
CDISDK = $(subst \,/,$(CDI_SDK_PATH))
OS9C   = $(CDISDK)/dos/bin
OS68K  = $(CDISDK)/OS9/68000
OS9CDI = $(CDISDK)/OS9/CDI
XCLIB  = $(OS68K)/lib
XCDEF  = $(OS68K)/defs
CLIB   = $(OS9CDI)/lib
CDEF   = $(OS9CDI)/defs
ANSIDEFS = $(CDISDK)/SRC/DEFS
DOSBOX = $(subst \,/,$(CDI_DOSBOX_PATH))/DOSBox.exe
MASTERTOOLS = $(CDISDK)/TOOLS/Master

# compiler flags and settings
CC       = $(OS9C)/xcc.exe
CCFLAGS  = -tp=68K,sc -v=$(CDEF) -v=$(SRC) -v=$(GAME) -v=$(ROOMS) -r -cq -dDISABLE_INTERNAL_DRAWING
ASFLAGS  = -O=0 -S -R=$(OUTPUT) -T=$(OUTPUT) -TO=osk -TP=68kI
LD       = $(OS9C)/l68
FIXMOD   = $(OS9C)/fixmod

# helpers for turning LDPARAM into a one-token-per-line linker response file
empty :=
space := $(empty) $(empty)
define newline


endef

# sources
SOURCES      = $(wildcard $(SRC)/*.c)
ASM_SOURCES  = $(wildcard $(SRC)/*.a) # .a files not .asm because reasons
DRAW_ERASE_SOURCES     = $(wildcard $(DRAW_ERASE)/*.c)
DRAW_ERASE_ASM_SOURCES = $(wildcard $(DRAW_ERASE)/*.a)
CDI_HW_SOURCES = $(wildcard $(CDI_HW)/*.c)
GAME_SOURCES = $(wildcard $(GAME)/*.c)
ROOM_SOURCES = $(wildcard $(ROOMS)/*.c)

# objects
SRC_OBJS  = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.r,$(SOURCES))
ASM_OBJS  = $(patsubst $(SRC)/%.a,$(OUTPUT)/%.r,$(ASM_SOURCES))
DRAW_ERASE_OBJS     = $(patsubst $(DRAW_ERASE)/%.c,$(OUTPUT)/draw_erase/%.r,$(DRAW_ERASE_SOURCES))
DRAW_ERASE_ASM_OBJS = $(patsubst $(DRAW_ERASE)/%.a,$(OUTPUT)/draw_erase/%.r,$(DRAW_ERASE_ASM_SOURCES))
CDI_HW_OBJS = $(patsubst $(CDI_HW)/%.c,$(OUTPUT)/cdi_hw/%.r,$(CDI_HW_SOURCES))
GAME_OBJS = $(patsubst $(GAME)/%.c,$(OUTPUT)/game/%.r,$(GAME_SOURCES))
ROOM_OBJS = $(patsubst $(ROOMS)/%.c,$(OUTPUT)/game/rooms/%.r,$(ROOM_SOURCES))
OBJS      = $(SRC_OBJS) $(ASM_OBJS) $(DRAW_ERASE_OBJS) $(DRAW_ERASE_ASM_OBJS) $(CDI_HW_OBJS) $(GAME_OBJS) $(ROOM_OBJS)

# linker setup
LDPARAM = -a -n=cdi_$(NAME) -o=$(OUTPUT)/$(NAME) \
          $(CLIB)/cstart.r \
          $(OBJS) \
          -l=$(CLIB)/cdi.l \
          -l=$(CLIB)/cdisys.l \
          -l=$(CLIB)/clib.l \
          -l=$(CLIB)/cio.l \
          -l=$(CLIB)/math.l \
          -l=$(CLIB)/sys.l \
          -l=$(CLIB)/usr.l \
          -l=$(XCLIB)/os_csl.l

# build targets
cd: link
	mkdir -p $(DISK)
	$(DOSBOX) -c "mount c ." -c "mount d $(MASTERTOOLS)" -c "c:" \
	          -c "config -set cycles max" \
	          -c "PATH %PATH%;D:" -c "vcdmastr.exe build\master.cd > $(OUTPUT)\master.log" \
	          -c "exit"

all: link

rebuild: clean cd

link: $(OBJS)
	$(file >$(OUTPUT)/link.txt,$(subst $(space),$(newline),$(strip $(LDPARAM))))
	$(LD) -z=./$(OUTPUT)/link.txt
	$(FIXMOD) -uo=0.0 $(OUTPUT)/$(NAME)


# compile log.c which needs -bc to call printf() directly
$(OUTPUT)/log.r: $(SRC)/log.c
	$(CC) -eas=$(OUTPUT) $(CCFLAGS) -bc -v=$(ANSIDEFS) -O=2 $<

# regular .c files
$(OUTPUT)/%.r: $(SRC)/%.c
	$(CC) -eas=$(OUTPUT) $(CCFLAGS) -O=2 $<

$(ASM_OBJS): $(OUTPUT)/%.r: $(SRC)/%.a
	$(CC) -eas=$(OUTPUT) $(CCFLAGS) -O=2 $<

$(DRAW_ERASE_OBJS): $(OUTPUT)/draw_erase/%.r: $(DRAW_ERASE)/%.c | $(OUTPUT)/draw_erase
	$(CC) -eas=$(OUTPUT)/draw_erase $(CCFLAGS) -O=2 $<

$(DRAW_ERASE_ASM_OBJS): $(OUTPUT)/draw_erase/%.r: $(DRAW_ERASE)/%.a | $(OUTPUT)/draw_erase
	$(CC) -eas=$(OUTPUT)/draw_erase $(CCFLAGS) -O=2 $<

$(CDI_HW_OBJS): $(OUTPUT)/cdi_hw/%.r: $(CDI_HW)/%.c | $(OUTPUT)/cdi_hw
	$(CC) -eas=$(OUTPUT)/cdi_hw $(CCFLAGS) -O=2 $<

$(OUTPUT)/game/%.r: $(GAME)/%.c | $(OUTPUT)/game
	$(CC) -eas=$(OUTPUT)/game $(CCFLAGS) -O=2 $<

$(OUTPUT)/game/rooms/%.r: $(ROOMS)/%.c | $(OUTPUT)/game/rooms
	$(CC) -eas=$(OUTPUT)/game/rooms $(CCFLAGS) -O=2 $<

# setup output directories
$(OUTPUT)/draw_erase:
	mkdir -p $@

$(OUTPUT)/cdi_hw:
	mkdir -p $@

$(OUTPUT)/game:
	mkdir -p $@

$(OUTPUT)/game/rooms:
	mkdir -p $@

# clean
clean:
	-rm -rf $(OUTPUT)/*
