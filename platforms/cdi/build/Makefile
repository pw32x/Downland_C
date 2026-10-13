NAME    = Downland
OUTPUT  = out
GAMEDATA = gamedata
SRC      = src
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
CCFLAGS  = -tp=68K,sc -v=$(CDEF) -v=$(GAME) -v=$(ROOMS) -r -cq -dDISABLE_INTERNAL_DRAWING
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
GAME_SOURCES = $(wildcard $(GAME)/*.c)
ROOM_SOURCES = $(wildcard $(ROOMS)/*.c)

# objects
SRC_OBJS  = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.r,$(SOURCES))
ASM_OBJS  = $(patsubst $(SRC)/%.a,$(OUTPUT)/%.r,$(ASM_SOURCES))
GAME_OBJS = $(patsubst $(GAME)/%.c,$(OUTPUT)/game/%.r,$(GAME_SOURCES))
ROOM_OBJS = $(patsubst $(ROOMS)/%.c,$(OUTPUT)/game/rooms/%.r,$(ROOM_SOURCES))
OBJS      = $(SRC_OBJS) $(ASM_OBJS) $(GAME_OBJS) $(ROOM_OBJS)

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

$(OUTPUT)/game/%.r: $(GAME)/%.c | $(OUTPUT)/game
	$(CC) -eas=$(OUTPUT)/game $(CCFLAGS) -O=2 $<

$(OUTPUT)/game/rooms/%.r: $(ROOMS)/%.c | $(OUTPUT)/game/rooms
	$(CC) -eas=$(OUTPUT)/game/rooms $(CCFLAGS) -O=2 $<

# setup output directories
$(OUTPUT)/game:
	mkdir -p $@

$(OUTPUT)/game/rooms:
	mkdir -p $@

# clean
clean:
	-rm -rf $(OUTPUT)/*
