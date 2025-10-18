#change to your preferred name
#pass the PROGNAME name as a parameter to set a new rom name. "make PROGNAME=my_new_name"
PROGNAME?=Downland

#change to your devkitSMS folder
DEVKITSMS_BASE=../../../../../SMSDev/devkitSMS-master

CC=sdcc
IHX2SMS=$(DEVKITSMS_BASE)/ihx2sms/Linux/ihx2sms
SMSLIB_BASE=$(DEVKITSMS_BASE)/SMSlib
PSGLIB_BASE=$(DEVKITSMS_BASE)/PSGlib
SMSLIB_INCDIR=$(SMSLIB_BASE)/src
PSGLIB_INCDIR=$(PSGLIB_BASE)/src
PEEP_RULES=$(SMSLIB_BASE)/src/peep-rules.txt
CRT0=$(DEVKITSMS_BASE)/crt0/crt0_sms.rel
SMSLIB_LIB=$(SMSLIB_BASE)/SMSlib.lib          # Use distributed lib
PSGLIB_LIB=$(PSGLIB_BASE)/PSGlib.lib          # Use distributed lib
#SMSLIB_LIB=$(SMSLIB_BASE)/src/SMSlib.lib      # Use locally compiled lib

CFLAGS=-mz80 -I$(SMSLIB_INCDIR) -I$(PSGLIB_INCDIR) -I$(GENERATED_DIR) -I$(SOURCE_DIR) -I$(GAME_SRC_DIR) -I$(GAME_ROOMS_SRC_DIR) --peep-file $(PEEP_RULES) \
		--opt-code-speed --peep-asm 

LDFLAGS=-mz80 --no-std-crt0 --data-loc 0xC000

SOURCE_DIR = source
GENERATED_DIR = generated

# New source locations for game code
GAME_SRC_DIR   = ../../game_8bit
GAME_ROOMS_SRC_DIR = ../../game_8bit/rooms

GENERATED_BANK2_DIR = generated/bank2
GENERATED_BANK3_DIR = generated/bank3
GENERATED_BANK4_DIR = generated/bank4
GENERATED_BANK5_DIR = generated/bank5
GENERATED_BANK6_DIR = generated/bank6
GENERATED_BANK7_DIR = generated/bank7
GENERATED_SPRITES_DIR = generated/sprites

OUTPUT_DIR = out

SOURCE_OUTPUT_DIR = $(OUTPUT_DIR)/$(SOURCE_DIR)
GENERATED_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_DIR)
GENERATED_BANK2_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_BANK2_DIR)
GENERATED_BANK3_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_BANK3_DIR)
GENERATED_BANK4_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_BANK4_DIR)
GENERATED_BANK5_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_BANK5_DIR)
GENERATED_BANK6_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_BANK6_DIR)
GENERATED_BANK7_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_BANK7_DIR)
GENERATED_SPRITES_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_SPRITES_DIR)
GAME_OUTPUT_DIR   = $(OUTPUT_DIR)/$(SOURCE_DIR)/game
ROOMS_OUTPUT_DIR  = $(OUTPUT_DIR)/$(SOURCE_DIR)/game/rooms

# collect sources
CS					= $(wildcard $(SOURCE_DIR)/*.c)
GENERATED_CS		= $(wildcard $(GENERATED_DIR)/*.c)
GENERATED_BANK2_CS  = $(wildcard $(GENERATED_BANK2_DIR)/*.c)
GENERATED_BANK3_CS  = $(wildcard $(GENERATED_BANK3_DIR)/*.c)
GENERATED_BANK4_CS  = $(wildcard $(GENERATED_BANK4_DIR)/*.c)
GENERATED_BANK5_CS  = $(wildcard $(GENERATED_BANK5_DIR)/*.c)
GENERATED_BANK6_CS  = $(wildcard $(GENERATED_BANK6_DIR)/*.c)
GENERATED_BANK7_CS  = $(wildcard $(GENERATED_BANK7_DIR)/*.c)
GENERATED_SPRITES_CS  = $(wildcard $(GENERATED_SPRITES_DIR)/*.c)
GAME_CS				= $(wildcard $(GAME_SRC_DIR)/*.c)
ROOMS_CS			= $(wildcard $(GAME_ROOMS_SRC_DIR)/*.c)

# map sources to objects
SOURCE_OBJS				= $(patsubst $(SOURCE_DIR)/%.c,$(SOURCE_OUTPUT_DIR)/%.rel,$(CS))
GENERATED_OBJS			= $(patsubst $(GENERATED_DIR)/%.c,$(GENERATED_OUTPUT_DIR)/%.rel,$(GENERATED_CS))
GENERATED_BANK2_OBJS	= $(patsubst $(GENERATED_BANK2_DIR)/%.c,$(GENERATED_BANK2_OUTPUT_DIR)/%.rel,$(GENERATED_BANK2_CS))
GENERATED_BANK3_OBJS	= $(patsubst $(GENERATED_BANK3_DIR)/%.c,$(GENERATED_BANK3_OUTPUT_DIR)/%.rel,$(GENERATED_BANK3_CS))
GENERATED_BANK4_OBJS	= $(patsubst $(GENERATED_BANK4_DIR)/%.c,$(GENERATED_BANK4_OUTPUT_DIR)/%.rel,$(GENERATED_BANK4_CS))
GENERATED_BANK5_OBJS	= $(patsubst $(GENERATED_BANK5_DIR)/%.c,$(GENERATED_BANK5_OUTPUT_DIR)/%.rel,$(GENERATED_BANK5_CS))
GENERATED_BANK6_OBJS	= $(patsubst $(GENERATED_BANK6_DIR)/%.c,$(GENERATED_BANK6_OUTPUT_DIR)/%.rel,$(GENERATED_BANK6_CS))
GENERATED_BANK7_OBJS	= $(patsubst $(GENERATED_BANK7_DIR)/%.c,$(GENERATED_BANK7_OUTPUT_DIR)/%.rel,$(GENERATED_BANK7_CS))
GENERATED_SPRITES_OBJS	= $(patsubst $(GENERATED_SPRITES_DIR)/%.c,$(GENERATED_SPRITES_OUTPUT_DIR)/%.rel,$(GENERATED_SPRITES_CS))
GAME_OBJS				= $(patsubst $(GAME_SRC_DIR)/%.c,$(GAME_OUTPUT_DIR)/%.rel,$(GAME_CS))
ROOMS_OBJS				= $(patsubst $(GAME_ROOMS_SRC_DIR)/%.c,$(ROOMS_OUTPUT_DIR)/%.rel,$(ROOMS_CS))

OBJS = $(SOURCE_OBJS) $(GENERATED_OBJS) $(GAME_OBJS) $(ROOMS_OBJS) \
       $(GENERATED_BANK2_OBJS) $(GENERATED_BANK3_OBJS) $(GENERATED_BANK4_OBJS) \
       $(GENERATED_BANK5_OBJS) $(GENERATED_BANK6_OBJS) $(GENERATED_BANK7_OBJS) \
       $(GENERATED_SPRITES_OBJS)

all: directories $(OUTPUT_DIR)/$(PROGNAME).sms

directories:
	mkdir -p $(SOURCE_OUTPUT_DIR)
	mkdir -p $(GENERATED_OUTPUT_DIR)
	mkdir -p $(GENERATED_BANK2_OUTPUT_DIR)
	mkdir -p $(GENERATED_BANK3_OUTPUT_DIR)
	mkdir -p $(GENERATED_BANK4_OUTPUT_DIR)
	mkdir -p $(GENERATED_BANK5_OUTPUT_DIR)
	mkdir -p $(GENERATED_BANK6_OUTPUT_DIR)
	mkdir -p $(GENERATED_BANK7_OUTPUT_DIR)
	mkdir -p $(GENERATED_SPRITES_OUTPUT_DIR)
	mkdir -p $(GAME_OUTPUT_DIR)
	mkdir -p $(ROOMS_OUTPUT_DIR)

# compile rules
$(SOURCE_OUTPUT_DIR)/%.rel: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(GENERATED_OUTPUT_DIR)/%.rel: $(GENERATED_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(GENERATED_BANK2_OUTPUT_DIR)/%.rel: $(GENERATED_BANK2_DIR)/%.c
	$(CC) $(CFLAGS) --constseg BANK2 -c $< -o $@ 

$(GENERATED_BANK3_OUTPUT_DIR)/%.rel: $(GENERATED_BANK3_DIR)/%.c
	$(CC) $(CFLAGS) --constseg BANK3 -c $< -o $@ 

$(GENERATED_BANK4_OUTPUT_DIR)/%.rel: $(GENERATED_BANK4_DIR)/%.c
	$(CC) $(CFLAGS) --constseg BANK4 -c $< -o $@ 

$(GENERATED_BANK5_OUTPUT_DIR)/%.rel: $(GENERATED_BANK5_DIR)/%.c
	$(CC) $(CFLAGS) --constseg BANK5 -c $< -o $@ 

$(GENERATED_BANK6_OUTPUT_DIR)/%.rel: $(GENERATED_BANK6_DIR)/%.c
	$(CC) $(CFLAGS) --constseg BANK6 -c $< -o $@ 

$(GENERATED_BANK7_OUTPUT_DIR)/%.rel: $(GENERATED_BANK7_DIR)/%.c
	$(CC) $(CFLAGS) --constseg BANK7 -c $< -o $@ 

$(GENERATED_SPRITES_OUTPUT_DIR)/%.rel: $(GENERATED_SPRITES_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(GAME_OUTPUT_DIR)/%.rel: $(GAME_SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ROOMS_OUTPUT_DIR)/%.rel: $(GAME_ROOMS_SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/$(PROGNAME).ihx: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) \
	-Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 -Wl-b_BANK4=0x8000 \
	-Wl-b_BANK5=0x8000 -Wl-b_BANK6=0x8000 -Wl-b_BANK7=0x8000 \
	$(CRT0) $(SMSLIB_LIB) $(PSGLIB_LIB) $^

$(OUTPUT_DIR)/$(PROGNAME).sms: $(OUTPUT_DIR)/$(PROGNAME).ihx
	$(IHX2SMS) $< $@

clean:
	rm -f $(SOURCE_OUTPUT_DIR)/*.rel 	
	rm -f $(SOURCE_OUTPUT_DIR)/*.asm 
	rm -f $(SOURCE_OUTPUT_DIR)/*.sym 
	rm -f $(SOURCE_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_BANK2_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_BANK2_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_BANK2_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_BANK2_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_BANK3_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_BANK3_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_BANK3_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_BANK3_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_BANK4_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_BANK4_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_BANK4_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_BANK4_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_BANK5_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_BANK5_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_BANK5_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_BANK5_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_BANK6_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_BANK6_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_BANK6_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_BANK6_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_BANK7_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_BANK7_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_BANK7_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_BANK7_OUTPUT_DIR)/*.lst 
	rm -f $(GENERATED_SPRITES_OUTPUT_DIR)/*.rel 	
	rm -f $(GENERATED_SPRITES_OUTPUT_DIR)/*.asm 
	rm -f $(GENERATED_SPRITES_OUTPUT_DIR)/*.sym 
	rm -f $(GENERATED_SPRITES_OUTPUT_DIR)/*.lst 
	rm -f $(GAME_OUTPUT_DIR)/*.rel 	
	rm -f $(GAME_OUTPUT_DIR)/*.asm 
	rm -f $(GAME_OUTPUT_DIR)/*.sym 
	rm -f $(GAME_OUTPUT_DIR)/*.lst 
	rm -f $(ROOMS_OUTPUT_DIR)/*.rel 	
	rm -f $(ROOMS_OUTPUT_DIR)/*.asm 
	rm -f $(ROOMS_OUTPUT_DIR)/*.sym 
	rm -f $(ROOMS_OUTPUT_DIR)/*.lst 
	rm -f $(OUTPUT_DIR)/*.ihx 
	rm -f $(OUTPUT_DIR)/*.noi 
	rm -f $(OUTPUT_DIR)/*.lk 
	rm -f $(OUTPUT_DIR)/*.map 
	rm -f $(OUTPUT_DIR)/*.sms 
	rm -f $(OUTPUT_DIR)/*.gg

# When a linking error occurs, sdcc returns an error and make fails. However
# a .ihx output file still gets created. This leads to make thinking that the .ihx
# file is up to date the next time it runs... This forces linking to take place
# every time make is called.
.PHONY: $(PROGNAME).ihx directories
