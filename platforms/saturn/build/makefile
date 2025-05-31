ifndef SRL_SDK_ROOT
$(error Environment variable SRL_SDK_ROOT is not set)
endif

# Configuration
SRL_MAX_TEXTURES = 100          # Number of VDP1 texture slots
SRL_MODE = NTSC                 # Valid options are PAL or NTSC
SRL_HIGH_RES = 0                # 480i mode
SRL_FRAMERATE = 1               # Framerate control (0=dynamic, 1=< 60/value)
SRL_MAX_CD_BACKGROUND_JOBS = 1  # Maximum number of files GFS can open at once
SRL_MAX_CD_FILES = 256          # Maximum number of files on a CD
SRL_MAX_CD_RETRIES = 5          # Number of times to retry on unsuccessful read

# Sound driver specific configuration
SRL_USE_SGL_SOUND_DRIVER = 0    # Set to 1 if you want to use SGL sound driver, this will copy necessary files into the CD folder
SRL_ENABLE_FREQ_ANALYSIS = 0    # Set to 1 if you want to enable frequency analysis for CD audio, this will load a DSP program into effect slot 1, SGL sound driver must be enabled

# SGL configuration
SGL_MAX_VERTICES = 2500         # Number of vertices that can be used
SGL_MAX_POLYGONS = 1500         # Number of polygons that can be used
SGL_MAX_EVENTS = 1             	# Number of events that can be used
SGL_MAX_WORKS = 1             	# Number of works that can be used 

# Disk name
CD_NAME = Downland

SRC_ROOT = .
GAME_ROOT = $(SRC_ROOT)/../../game
ROOMS_ROOT = $(GAME_ROOT)/rooms

OUT_DIR = ./out

# Find all .c and .cpp files
LOCAL_SRCS = $(wildcard $(SRC_ROOT)/source/*.c)
LOCAL_SRCS += $(wildcard $(SRC_ROOT)/source/*.cpp)

GAME_SRCS = $(wildcard $(GAME_ROOT)/*.c)
ROOMS_SRCS = $(wildcard $(ROOMS_ROOT)/*.c)

COMPILER_DIR=$(SRL_SDK_ROOT)/../Compiler

MODDIR = $(SRL_SDK_ROOT)/../modules
CMODIR = $(COMPILER_DIR)/Other Utilities

STDDIR = $(MODDIR)/danny/INC

SGLDIR = $(MODDIR)/sgl
IPFILE = $(SGLDIR)/IP.BIN
LDFILE = $(SGLDIR)/sgl.linker
SGLLDIR = $(SGLDIR)/LIB
SGLIDIR = $(SGLDIR)/INC
SGLSDIR = $(SGLDIR)/SRC

LIBS = $(SGLLDIR)/LIBCPK.A $(SGLLDIR)/SEGA_SYS.A $(SGLLDIR)/LIBCD.A $(SGLLDIR)/LIBSGL.A

# include extra modules
MODULE_EXTRA_INC =

.DEFAULT_GOAL := all

ifneq ($(strip $(MODULES_EXTRA)),)
	include $(patsubst %, $(SRL_SDK_ROOT)/../modules_extra/%/module.mk, $(strip $(MODULES_EXTRA)))
	MODULE_EXTRA_INC += $(patsubst %, -I$(SRL_SDK_ROOT)/../modules_extra/%/INC, $(strip $(MODULES_EXTRA)))
	MODULE_OBJECTS = $(MODULE_SOURCES:.c=.o)
	OBJECTS += $(MODULE_OBJECTS:.cpp=.o)
endif

COBJECTS = $(patsubst $(SRC_ROOT)/source/%.c,$(OUT_DIR)/source/%.o,$(LOCAL_SRCS))
LOCAL_OBJS += $(patsubst $(SRC_ROOT)/source/%.cpp,$(OUT_DIR)/source/%.o,$(LOCAL_SRCS))
GAME_OBJS = $(patsubst $(GAME_ROOT)/%.c,$(OUT_DIR)/game/%.o,$(GAME_SRCS))
ROOMS_OBJS = $(patsubst $(ROOMS_ROOT)/%.c,$(OUT_DIR)/game/rooms/%.o,$(ROOMS_SRCS))

OBJECTS = $(LOCAL_OBJS) $(GAME_OBJS) $(ROOMS_OBJS) $(ROMDISK_OBJ)


ifdef OS
   CC = sh2eb-elf-gcc.exe
   CPP = sh2eb-elf-g++.exe
   LD = sh2eb-elf-gcc.exe
   OBJCOPY = sh2eb-elf-objcopy.exe
else
	ifneq (, $(shell which sh2eb-elf-gcc))
		CC = sh2eb-elf-gcc
		CPP = sh2eb-elf-g++
		LD = sh2eb-elf-gcc
		OBJCOPY = sh2eb-elf-objcopy
	else
		CC = sh-elf-gcc
		CPP = sh-elf-g++
		LD = sh-elf-gcc
		OBJCOPY = sh-elf-objcopy
  endif
endif

GCCMAJORVERSION = $(shell $(CC) --version | head -n 1 | sed 's/^.* //g' | sed 's/[^0-9].//g')

# Handle defaults for user settings
ifeq ($(strip ${SRL_MAX_CD_BACKGROUND_JOBS}),)
	SRL_MAX_CD_BACKGROUND_JOBS=1
endif

ifeq ($(strip ${SRL_MAX_CD_FILES}),)
	SRL_MAX_CD_FILES=255
endif

ifeq ($(strip ${SRL_MAX_CD_RETRIES}),)
	SRL_MAX_CD_RETRIES=5
endif

ifeq ($(strip ${SRL_HIGH_RES}), 1)
	CCFLAGS += -DSRL_HIGH_RES
endif

ifeq ($(strip ${SRL_FRAMERATE}),)
	CCFLAGS += -DSRL_FRAMERATE=0
else
	CCFLAGS += -DSRL_FRAMERATE=$(strip ${SRL_FRAMERATE})
endif

ifeq ($(strip ${SRL_MAX_TEXTURES}),)
	SRL_MAX_TEXTURES=100
endif

ifeq ($(strip ${DEBUG}), 1)
	CCFLAGS += -DDEBUG
endif

ifneq ($(strip ${SRL_LOG_LEVEL}),)
	CCFLAGS += -DSRL_LOG_LEVEL=$(strip ${SRL_LOG_LEVEL})
endif

ifeq ($(strip ${SRL_USE_SGL_SOUND_DRIVER}), 1)
	CCFLAGS += -DSRL_USE_SGL_SOUND_DRIVER=$(strip ${SRL_USE_SGL_SOUND_DRIVER})
	LIBS += $(SGLLDIR)/LIBSND.A

	ifeq ($(strip ${SRL_ENABLE_FREQ_ANALYSIS}),1)
		CCFLAGS += -DSRL_ENABLE_FREQ_ANALYSIS=1
	endif
endif

ifneq ($(strip ${SRL_MODE}),PAL)
	ifneq (${SRL_MODE},NTSC)
		SRL_MODE = NTSC
	endif
endif

ifeq ($(strip ${SRL_DEBUG_MAX_PRINT_LENGTH}),)
	SRL_DEBUG_MAX_PRINT_LENGTH = 45
endif

ifeq ($(strip ${SRL_DEBUG_MAX_LOG_LENGTH}),)
	SRL_DEBUG_MAX_LOG_LENGTH = 80
endif

CCFLAGS += -DSRL_MODE_$(strip ${SRL_MODE}) \
	-DSRL_MAX_TEXTURES=$(strip ${SRL_MAX_TEXTURES}) \
	-DSRL_MAX_CD_BACKGROUND_JOBS=$(strip ${SRL_MAX_CD_BACKGROUND_JOBS}) \
	-DSRL_MAX_CD_FILES=$(strip ${SRL_MAX_CD_FILES}) \
	-DSRL_MAX_CD_RETRIES=$(strip ${SRL_MAX_CD_RETRIES}) \
	-DSRL_DEBUG_MAX_PRINT_LENGTH=$(strip ${SRL_DEBUG_MAX_PRINT_LENGTH}) \
	-DSRL_DEBUG_MAX_LOG_LENGTH=$(strip ${SRL_DEBUG_MAX_LOG_LENGTH}) \

# CD assets
ASSETS_DIR = ./cd/data
MUSIC_DIR = ./cd/music

# Handle work area
ifneq ($(strip ${SGL_MAX_VERTICES}),)
	SYSFLAGS += -DSGL_MAX_VERTICES=$(strip ${SGL_MAX_VERTICES})
else
	SYSFLAGS += -DSGL_MAX_VERTICES=2500
endif

ifneq ($(strip ${SGL_MAX_POLYGONS}),)
	SYSFLAGS += -DSGL_MAX_POLYGONS=$(strip ${SGL_MAX_POLYGONS})
else
	SYSFLAGS += -DSGL_MAX_POLYGONS=1700
endif

ifneq ($(strip ${SGL_MAX_EVENTS}),)
	SYSFLAGS += -DSGL_MAX_EVENTS=$(strip ${SGL_MAX_EVENTS})
else
	SYSFLAGS += -DSGL_MAX_EVENTS=64
endif

ifneq ($(strip ${SGL_MAX_WORKS}),)
	SYSFLAGS += -DSGL_MAX_WORKS=$(strip ${SGL_MAX_WORKS})
else
	SYSFLAGS += -DSGL_MAX_WORKS=256
endif

# Add custom FLAGS
ifneq ($(strip ${SRL_CUSTOM_CCFLAGS}),)
	CCFLAGS += $(strip ${SRL_CUSTOM_CCFLAGS})
endif

BUILD_ELF = $(OUT_DIR)/$(CD_NAME).elf
BUILD_ISO = $(BUILD_ELF:.elf=.iso)
BUILD_BIN = $(BUILD_ELF:.elf=.bin)
BUILD_CUE = $(BUILD_ELF:.elf=.cue)
BUILD_MAP = $(BUILD_ELF:.elf=.map)

TLSFDIR = $(MODDIR)/tlsf
DUMMYIDIR = $(MODDIR)/dummy
SATURNMATHPPDIR = $(MODDIR)/SaturnMathPP

SYSSOURCES += $(SGLSDIR)/workarea.c

ifdef SRL_MALLOC_METHOD
	ifeq ($(SRL_MALLOC_METHOD), TLSF)
		SYSSOURCES += $(TLSFDIR)/tlsf.c
		USE_TLSF_ALLOCATOR := TRUE
	endif
endif

SYSOBJECTS = $(patsubst $(SGLSDIR)/%.c,$(OUT_DIR)/modules/sgl/SRC/%.o,$(SYSSOURCES))

# General compilation flags
CCFLAGS += $(SYSFLAGS) -W -m2 -c -O2 -Wno-strict-aliasing \
					-I$(DUMMYIDIR) \
					-I$(SATURNMATHPPDIR) \
					-I$(SGLIDIR) \
					-I$(STDDIR) \
					-I$(TLSFDIR) \
					-I$(SRL_SDK_ROOT) \
					$(MODULE_EXTRA_INC) \
					-I$(GAME_ROOT) \
					-I$(ROOMS_ROOT)
LDFLAGS = -m2 -L$(SGLLDIR) -Xlinker -T$(LDFILE) -Xlinker -Map \
					-Xlinker $(BUILD_MAP) -Xlinker -e -Xlinker ___Start -nostartfiles

ifeq "$(GCCMAJORVERSION)" "14"
    LDFLAGS += -specs=nosys.specs
endif

# Add custom FLAGS
ifneq ($(strip ${SRL_CUSTOM_LDFLAGS}),)
	LDFLAGS += $(strip ${SRL_CUSTOM_LDFLAGS})
endif

# pre-build script
ifneq ("$(wildcard ./pre.makefile)","")
	include ./pre.makefile
else
pre_build:
	$(info ****** No pre build steps ******)
endif

# post-build script
ifneq ("$(wildcard ./post.makefile)","")
	include ./post.makefile
else
post_build:
	$(info ****** No post build steps ******)
endif

# Compilation tasks
$(OUT_DIR)/source/%.o : $(SRC_ROOT)/source/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(CCFLAGS) -std=c2x -o $@

$(OUT_DIR)/source/%.o : $(SRC_ROOT)/source/%.cpp
	@mkdir -p $(dir $@)
	$(CPP) -c $< $(CCFLAGS) -std=c++23 -fpermissive -fno-exceptions -fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-threadsafe-statics -fno-use-cxa-atexit -o $@

$(OUT_DIR)/game/%.o: $(GAME_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(CCFLAGS) -std=c2x -o $@
	
$(OUT_DIR)/game/rooms/%.o: $(ROOMS_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(CCFLAGS) -std=c2x -o $@


$(OUT_DIR)/modules/sgl/SRC/%.o: $(SGLSDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(CCFLAGS) -std=c2x -o $@

compile_objects : $(OBJECTS) $(SYSOBJECTS)
	$(info ****** Info ******)
	$(info Maximum textures : ${SRL_MAX_TEXTURES})
	$(info Maximum vertices : ${SGL_MAX_VERTICES})
	$(info Maximum polygons : ${SGL_MAX_POLYGONS})
	$(info Maximum events : ${SGL_MAX_EVENTS})
	$(info Maximum work : ${SGL_MAX_WORKS})
	$(info Log level selected : $(if $(strip ${SRL_LOG_LEVEL}),${SRL_LOG_LEVEL},NONE))
	$(info Maximum Log length : $(if $(strip ${SRL_DEBUG_MAX_LOG_LENGTH}),${SRL_DEBUG_MAX_LOG_LENGTH},0))
	$(info ******************)
	mkdir -p $(MUSIC_DIR)
	mkdir -p $(ASSETS_DIR)
	mkdir -p $(OUT_DIR)
	test -f $(ASSETS_DIR)/ABS.TXT || echo "NOT Abstracted by SEGA" >> $(ASSETS_DIR)/ABS.TXT
	test -f $(ASSETS_DIR)/BIB.TXT || echo "NOT Bibliographiced by SEGA" >> $(ASSETS_DIR)/BIB.TXT
	test -f $(ASSETS_DIR)/CPY.TXT || touch $(ASSETS_DIR)/CPY.TXT
	$(CC) $(LDFLAGS) $(SYSOBJECTS) $(OBJECTS) $(LIBS) -o $(BUILD_ELF)

convert_binary : compile_objects
	$(OBJCOPY) -O binary $(BUILD_ELF) $(OUT_DIR)/0.bin

create_iso : convert_binary
ifeq ($(strip ${SRL_USE_SGL_SOUND_DRIVER}),1)
	cp -r $(SGLDIR)/DRV/. ./cd/data/
ifeq ($(strip ${SRL_ENABLE_FREQ_ANALYSIS}), 1)
	cp $(SGLDIR)/DSP/3BANDANA.EXB ./cd/data/
endif
endif
	xorrisofs --norock -quiet -sysid "SEGA SATURN" -volid "SaturnApp" -volset "SaturnApp" \
	-publisher "SEGA ENTERPRISES, LTD." -preparer "SEGA ENTERPRISES, LTD." -appid "SaturnApp" \
	-abstract "$(ASSETS_DIR)/ABS.TXT" -copyright "$(ASSETS_DIR)/CPY.TXT" -biblio "$(ASSETS_DIR)/BIB.TXT" -generic-boot $(IPFILE) \
	-full-iso9660-filenames -o $(BUILD_ISO) $(ASSETS_DIR) $(OUT_DIR)/0.bin $(ENTRYPOINT)

# Create CUE sheet
create_bin_cue: create_iso
	dd if=$(BUILD_ISO) of=$(BUILD_BIN) bs=2048
	echo 'FILE "$(CD_NAME).bin" BINARY' > $(BUILD_CUE)
	echo '  TRACK 01 MODE1/2048' >> $(BUILD_CUE)
	echo '    INDEX 01 00:00:00' >> $(BUILD_CUE)

AUDIO_FILES = $(patsubst ./%,%,$(shell find $(MUSIC_DIR) \( -name '*.mp3' -o -name '*.wav' -o -name '*.ogg' -o -name '*.flac' -o -name '*.aac' -o -name '*.m4a' -o -name '*.wma' \)))
AUDIO_FILES_RAW = $(patsubst %,%.raw,$(AUDIO_FILES))

%.raw: %
	sox $< -t raw -r 44100 -e signed-integer -b 16 -c 2 $@

add_audio_to_bin_cue: $(AUDIO_FILES_RAW)
	track=2; \
	total_size=$$(stat -c%s "$(BUILD_BIN)"); \
	for i in $^; do \
		sectors=$$((total_size / 2352)); \
		minutes=$$((sectors / (60 * 75))); \
		seconds=$$((sectors % (60 * 75) / 75)); \
		frames=$$((sectors % 75)); \
		echo '  TRACK' $$(printf "%02d" $$track) 'AUDIO' >> $(BUILD_CUE); \
		echo '    INDEX 01' $$(printf "%02d:%02d:%02d" $$minutes $$seconds $$frames) >> $(BUILD_CUE); \
		cat "$$i" >> $(BUILD_BIN); \
		track=$$((track + 1)); \
		size=$$(stat -c%s "$$i"); \
		total_size=$$((total_size + size)); \
	done
	rm -f $(AUDIO_FILES_RAW)

build_bin_cue: create_bin_cue add_audio_to_bin_cue


clean:
	rm -f $(SGLSDIR)/*.o
	rm -f $(OBJECTS) $(BUILD_ELF) $(BUILD_ISO) $(BUILD_MAP) $(ASSETS_DIR)/0.bin
	rm -f $(AUDIO_FILES_RAW)
ifeq ($(strip ${SRL_USE_SGL_SOUND_DRIVER}),1)
	rm -f $(ASSETS_DIR)/SDDRVS.DAT $(ASSETS_DIR)/SDDRVS.TSK $(ASSETS_DIR)/BOOTSND.MAP
ifeq ($(strip ${SRL_ENABLE_FREQ_ANALYSIS}), 1)
	rm -f $(ASSETS_DIR)/3BANDANA.EXB
endif
endif
	rm -rf $(OUT_DIR)/

build : pre_build build_bin_cue post_build

all: build
