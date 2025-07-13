NAME       = Downland
FILESYSTEM = cd_contents
LAUNCHME   = $(FILESYSTEM)/LaunchMe
STACKSIZE  = 4096
BANNER     = banner.png
OUTDIR     = out
ISONAME    = $(OUTDIR)/iso/$(NAME).iso

## Flag definitions ##
ifeq ($(DEBUG),1)
OPT      = -O0
DEFFLAGS = -DDEBUG=1
else
OPT      = -O2
DEFFLAGS = -DNDEBUG=1
endif

DEFFLAGS += -DDL_PLATFORM_3DO

TDO_DEVKIT_PATH = C:/3do-devkit

INCPATH  = ${TDO_DEVKIT_PATH}/include
INCFLAGS = -I$(INCPATH)/3do -I$(INCPATH)/community -I$(INCPATH)/ttl \
           -I../../game -I../../game/rooms
CFLAGS   = $(OPT) -bigend -za1 -zi4 -fa -fh -fx -fpu none -arch 3 -apcs "3/32/fp/swst/wide/softfp"
CXXFLAGS = $(CFLAGS)
ASFLAGS  = -bigend -fpu none -arch 3 -apcs "3/32/fp/swst"
LIBPATH  = ${TDO_DEVKIT_PATH}/lib
LDFLAGS  = -match 0x1 -nodebug -noscanlib -nozeropad -verbose -remove -aif -reloc -dupok -ro-base 0
STARTUP  = $(LIBPATH)/3do/cstartup.o

LIBS = \
	$(LIBPATH)/3do/3dlib.lib \
	$(LIBPATH)/3do/audio.lib \
	$(LIBPATH)/3do/clib.lib \
	$(LIBPATH)/3do/codec.lib \
	$(LIBPATH)/3do/compression.lib \
	$(LIBPATH)/3do/cpluslib.lib \
	$(LIBPATH)/3do/DataAcq.lib \
	$(LIBPATH)/3do/DataAcqShuttle.lib \
	$(LIBPATH)/3do/DS.lib \
	$(LIBPATH)/3do/DSShuttle.lib \
	$(LIBPATH)/3do/exampleslib.lib \
	$(LIBPATH)/3do/filesystem.lib \
	$(LIBPATH)/3do/graphics.lib \
	$(LIBPATH)/3do/input.lib \
	$(LIBPATH)/3do/international.lib \
	$(LIBPATH)/3do/intmath.lib \
	$(LIBPATH)/3do/lib3do.lib \
	$(LIBPATH)/3do/music.lib \
	$(LIBPATH)/3do/mvelib.lib \
	$(LIBPATH)/3do/operamath.lib \
	$(LIBPATH)/3do/pgl.lib \
	$(LIBPATH)/3do/string.lib \
	$(LIBPATH)/3do/Subscriber.lib \
	$(LIBPATH)/3do/swi.lib \
	$(LIBPATH)/community/cpplib.lib \
	$(LIBPATH)/community/example_folio.lib \
	$(LIBPATH)/community/svc_funcs.lib \
	$(LIBPATH)/community/svc_mem.lib

SRCS_S   = $(wildcard src/*.s)
SRCS_C   = $(wildcard src/*.c) $(wildcard ../../game/*.c) $(wildcard ../../game/rooms/*.c)
SRCS_CXX = $(wildcard src/*.cpp)

OBJS = \
  $(patsubst src/%.s,                    $(OUTDIR)/src/%.s.o,                $(SRCS_S)) \
  $(patsubst src/%.c,                    $(OUTDIR)/src/%.c.o,                $(filter src/%.c, $(SRCS_C))) \
  $(patsubst ../../game/%.c,             $(OUTDIR)/game/%.c.o,               $(filter ../../game/%.c, $(SRCS_C))) \
  $(patsubst ../../game/rooms/%.c,       $(OUTDIR)/game/rooms/%.c.o,         $(filter ../../game/rooms/%.c, $(SRCS_C))) \
  $(patsubst src/%.cpp,                  $(OUTDIR)/src/%.cpp.o,              $(SRCS_CXX))

DEPS = $(OBJS:.o=.d)

all: launchme modbin iso encrypt-iso

$(OUTDIR)/.touched:
	@mkdir -p $(OUTDIR)
	@touch $@

$(OUTDIR)/src/.touched:
	@mkdir -p $(OUTDIR)/src
	@touch $@

$(OUTDIR)/game/.touched:
	@mkdir -p $(OUTDIR)/game
	@touch $@

$(OUTDIR)/game/rooms/.touched:
	@mkdir -p $(OUTDIR)/game/rooms
	@touch $@

outdir: $(OUTDIR)/.touched $(OUTDIR)/src/.touched $(OUTDIR)/game/.touched $(OUTDIR)/game/rooms/.touched

objs: outdir $(OBJS)

$(LAUNCHME): objs
	armlink -o $@ $(LDFLAGS) $(STARTUP) $(LIBS) $(OBJS)

launchme: $(LAUNCHME)

modbin:
	modbin --name="$(NAME)" --time --stack=$(STACKSIZE) "$(LAUNCHME)" "$(LAUNCHME)"

banner:
	3it to-banner -o "$(FILESYSTEM)/BannerScreen" "$(BANNER)"

$(OUTDIR)/iso/.touched:
	@mkdir -p $(OUTDIR)/iso
	@touch $@

isodir: $(OUTDIR)/iso/.touched

iso: isodir
	3doiso -in "$(FILESYSTEM)" -out "$(ISONAME)"

encrypt-iso: $(ISONAME)
	3DOEncrypt genromtags "$(ISONAME)"

$(OUTDIR)/src/%.s.o: src/%.s
	armasm $(INCFLAGS) $(DEFFLAGS) $(ASFLAGS) $< -o $@

$(OUTDIR)/src/%.c.o: src/%.c
	armcc $(INCFLAGS) $(DEFFLAGS) $(CFLAGS) -M $< -o $@ > $(@:.o=.d)
	armcc $(INCFLAGS) $(DEFFLAGS) $(CFLAGS) -c $< -o $@

$(OUTDIR)/src/%.cpp.o: src/%.cpp
	armcpp $(INCFLAGS) $(DEFFLAGS) $(CXXFLAGS) -M $< -o $@ > $(@:.o=.d)
	armcpp $(INCFLAGS) $(DEFFLAGS) $(CXXFLAGS) -c $< -o $@

$(OUTDIR)/game/%.c.o: ../../game/%.c
	@mkdir -p $(dir $@)
	armcc $(INCFLAGS) $(DEFFLAGS) $(CFLAGS) -M $< -o $@ > $(@:.o=.d)
	armcc $(INCFLAGS) $(DEFFLAGS) $(CFLAGS) -c $< -o $@

$(OUTDIR)/game/rooms/%.c.o: ../../game/rooms/%.c
	@mkdir -p $(dir $@)
	armcc $(INCFLAGS) $(DEFFLAGS) $(CFLAGS) -M $< -o $@ > $(@:.o=.d)
	armcc $(INCFLAGS) $(DEFFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rvf "$(OUTDIR)" "iso" $(LAUNCHME)

run:
	run-iso "$(ISONAME)"

.PHONY: outdir isodir clean modbin banner iso encrypt-iso run

-include $(DEPS)
