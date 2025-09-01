#change to your preferred name
#pass the PROGNAME name as a parameter to set a new rom name. "make PROGNAME=my_new_name"
PROGNAME?=Downland

#change to your devkitSMS folder
DEVKITSMS_BASE=../../../../../SMSDev/devkitSMS-master

CC=sdcc
IHX2SMS=$(DEVKITSMS_BASE)/ihx2sms/Linux/ihx2sms
SMSLIB_BASE=$(DEVKITSMS_BASE)/SMSlib
SMSLIB_INCDIR=$(SMSLIB_BASE)/src
PEEP_RULES=$(SMSLIB_BASE)/src/peep-rules.txt
CRT0=$(DEVKITSMS_BASE)/crt0/crt0_sms.rel
SMSLIB_LIB=$(SMSLIB_BASE)/SMSlib.lib          # Use distributed lib
#SMSLIB_LIB=$(SMSLIB_BASE)/src/SMSlib.lib      # Use locally compiled lib

CFLAGS=-mz80 -I$(SMSLIB_INCDIR) -I$(GENERATED_DIR) -I$(SOURCE_DIR)/game -I$(SOURCE_DIR)/game/rooms --peep-file $(PEEP_RULES)
LDFLAGS=-mz80 --no-std-crt0 --data-loc 0xC000

SOURCE_DIR = source
GENERATED_DIR = generated
GAME_DIR   = $(SOURCE_DIR)/game
ROOMS_DIR  = $(SOURCE_DIR)/game/rooms

OUTPUT_DIR = out

SOURCE_OUTPUT_DIR = $(OUTPUT_DIR)/$(SOURCE_DIR)
GENERATED_OUTPUT_DIR = $(OUTPUT_DIR)/$(GENERATED_DIR)
GAME_OUTPUT_DIR   = $(OUTPUT_DIR)/$(SOURCE_DIR)/game
ROOMS_OUTPUT_DIR  = $(OUTPUT_DIR)/$(SOURCE_DIR)/game/rooms

# collect sources
CS				= $(wildcard $(SOURCE_DIR)/*.c)
GENERATED_CS    = $(wildcard $(GENERATED_DIR)/*.c)
GAME_CS			= $(wildcard $(GAME_DIR)/*.c)
ROOMS_CS		= $(wildcard $(ROOMS_DIR)/*.c)

# map sources to objects
SOURCE_OBJS		= $(patsubst $(SOURCE_DIR)/%.c,$(SOURCE_OUTPUT_DIR)/%.rel,$(CS))
GENERATED_OBJS	= $(patsubst $(GENERATED_DIR)/%.c,$(GENERATED_OUTPUT_DIR)/%.rel,$(GENERATED_CS))
GAME_OBJS		= $(patsubst $(GAME_DIR)/%.c,$(GAME_OUTPUT_DIR)/%.rel,$(GAME_CS))
ROOMS_OBJS		= $(patsubst $(ROOMS_DIR)/%.c,$(ROOMS_OUTPUT_DIR)/%.rel,$(ROOMS_CS))

OBJS = $(SOURCE_OBJS) $(GENERATED_OBJS) $(GAME_OBJS) $(ROOMS_OBJS)

all: directories $(OUTPUT_DIR)/$(PROGNAME).sms

directories:
	mkdir -p $(SOURCE_OUTPUT_DIR)
	mkdir -p $(GENERATED_OUTPUT_DIR)
	mkdir -p $(GAME_OUTPUT_DIR)
	mkdir -p $(ROOMS_OUTPUT_DIR)

# compile rules
$(SOURCE_OUTPUT_DIR)/%.rel: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(GENERATED_OUTPUT_DIR)/%.rel: $(GENERATED_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(GAME_OUTPUT_DIR)/%.rel: $(GAME_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ROOMS_OUTPUT_DIR)/%.rel: $(ROOMS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/$(PROGNAME).ihx: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(CRT0) $(SMSLIB_LIB) $^

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
