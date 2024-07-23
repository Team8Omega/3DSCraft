#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#
# NO_SMDH: if set to anything, no SMDH file is generated.
# ROMFS is the directory which contains the RomFS, relative to the Makefile (Optional)
# APP_TITLE is the name of the app stored in the SMDH file (Optional)
# APP_DESCRIPTION is the description of the app stored in the SMDH file (Optional)
# APP_AUTHOR is the author of the app stored in the SMDH file (Optional)
# ICON is the filename of the icon (.png), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.png
#     - icon.png
#     - <libctru folder>/default_icon.png
#---------------------------------------------------------------------------------

VERSION_MAJOR	:= 0
VERSION_MINOR	:= 5
VERSION_MICRO	:= 4

DEBUG			?=	1

TARGET			:=	3DSCraft
BUILD			:=	build
DATA			:=	data
META			:=	project
ROMFS			:=	romfs
INCLUDES		:=	lib include ../include 3DSCraft/include
SOURCES 		:= $(shell find $(realpath lib) $(realpath source) $(realpath assets) -type d)
SOURCES 		:= $(foreach dir, $(SOURCES), $(patsubst $(CURDIR)/%, %, $(dir)))

# 3dsx
APP_DESCRIPTION :=  3DSCraft
APP_AUTHOR		:=  Team-Omega
ICON			:=	$(META)/icon.png

# CIA
BANNER_AUDIO	:=	$(META)/banner.wav
BANNER_IMAGE	:=	$(META)/banner.cgfx
RSF_PATH		:=	$(META)/app.rsf
LOGO			:=	$(META)/logo.bcma.lz
ICON_FLAGS		:=	nosavebackups,visible


#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) -include $(DEVKITPRO)/libctru/include/3ds/types.h

CFLAGS	:=	-g -Wall -Wno-psabi -O2 -mword-relocations \
			-DC_V=\"$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)\" \
			-fomit-frame-pointer -ffunction-sections \
			$(ARCH) $(LDFLAGS)

CFLAGS	+=	$(INCLUDE) -D__3DS__ -D_3DS=1 -D_VER_MAJ=$(VERSION_MAJOR) -D_VER_MIN=$(VERSION_MINOR) -D_VER_MIC=$(VERSION_MICRO) -D_AUTHOR=$(APP_AUTHOR) -D_GNU_SOURCE=1
CFLAGS  +=  `$(PREFIX)pkg-config opusfile --cflags`

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS	:=	-g $(ARCH)

ifeq ($(DEBUG), 0)
CFLAGS		+=	-fomit-frame-pointer -O2
LIBS		:= -lcitro3d -lctru 
else
CFLAGS		+=	-Og -D_DEBUG
LIBS		:= -lcitro3dd -lctrud
endif

LIBS		+= -lm `$(PREFIX)pkg-config opusfile --libs`

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CURDIR) $(PORTLIBS) $(CTRULIB)



#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(PICAFILES:.v.pica=.shbin.o) $(SHLISTFILES:.shlist=.shbin.o) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_SMDH)),)
	export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh
endif

ifneq ($(ROMFS),)
	export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)
endif

#---------------------------------------------------------------------------------------
# Cia building preparation
#---------------------------------------------------------------------------------------
ifeq ($(UNAME_S),Linux)
BANNERTOOL   ?= tools/bannertool
MAKEROM      ?= tools/makerom
else
BANNERTOOL   ?= tools/bannertool.exe
MAKEROM      ?= tools/makerom.exe
endif

MAKEROM_ARGS += -elf "$(TARGET).elf" -rsf "$(RSF_PATH)" -banner "$(BUILD)/banner.bnr" -icon "$(TARGET).smdh"
MAKEROM_ARGS += -major $(VERSION_MAJOR) -minor $(VERSION_MINOR) -micro $(VERSION_MICRO) -desc app:4

ifneq ($(strip $(LOGO)),)
	MAKEROM_ARGS += -logo "$(LOGO)"
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD) cxi cfa cia

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#---------------------------------------------------------------------------------
clean: clean-lib
	@echo clean ...
	@rm -rf $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf $(TARGET).cia $(TARGET).cxi $(TARGET).cfa $(TARGET).lst
#---------------------------------------------------------------------------------
run:
	@echo running...
	@3dslink $(TARGET).3dsx
	
rund: #run dima
	@3dslink $(TARGET).3dsx -a 192.168.178.37

cxi:
	@$(MAKEROM) -o $(TARGET).cxi $(MAKEROM_ARGS)
	@echo built ... $(TARGET).cxi
cfa:
	@$(MAKEROM) -o $(TARGET).cfa -rsf $(RSF_PATH) -target t
	@echo built ... $(TARGET).cfa

cia:
	@$(MAKEROM) -f cia -o $(TARGET).cia -target t -i $(TARGET).cxi:0:0 -i $(TARGET).cfa:1:1
	@echo built ... $(TARGET).cia


#---------------------------------------------------------------------------------------
# Library
#---------------------------------------------------------------------------------------

#LIBSOURCES := $(wildcard lib/**/*.cpp lib/**/*.c)
#LIBSOURCES += $(wildcard lib/*/*/*.cpp lib/*/*/*.c)
#LIBOBJS := $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(LIBSOURCES)))

#AR := $(DEVKITARM)/bin/arm-none-eabi-ar.exe

lib: lib/libgame.a

lib/libgame.a: $(LIBOBJS)
	@echo Building libraries...
	@$(AR) rcs lib/libgame.a $^
	@echo built ... lib/libgame.a

#lib/%.o: lib/%.cpp
#	@echo $@...
#	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o: lib/%.c
	@echo $@...
	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean-lib:
	@rm -f lib/**/*.o
	@rm -f lib/libgame.o

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

ifeq ($(UNAME_S),Linux)
BANNERTOOL   ?= ../tools/bannertool
MAKEROM      ?= ../tools/makerom
else
BANNERTOOL   ?= ../tools/bannertool.exe
MAKEROM      ?= ../tools/makerom.exe
endif


ifeq ($(suffix $(BANNER_IMAGE)),.cgfx)
	BANNER_IMAGE_ARG := -ci
else
	BANNER_IMAGE_ARG := -i
endif

ifeq ($(suffix $(BANNER_AUDIO)),.cwav)
	BANNER_AUDIO_ARG := -ca
else
	BANNER_AUDIO_ARG := -a
endif

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
ifeq ($(strip $(NO_SMDH)),)
$(OUTPUT).3dsx	:	$(OUTPUT).elf $(OUTPUT).smdh $(BUILD)/banner.bnr
else
$(OUTPUT).3dsx	:	$(OUTPUT).elf $(BUILD)/banner.bnr
endif

$(OUTPUT).elf	:	$(OFILES)

$(BUILD)/banner.bnr:
	@$(BANNERTOOL) makebanner $(BANNER_IMAGE_ARG) "../$(BANNER_IMAGE)" $(BANNER_AUDIO_ARG) "../$(BANNER_AUDIO)" -o "banner.bnr"

$(OUTPUT).smdh:
	@$(BANNERTOOL) makesmdh -s "$(TARGET)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -i "$(APP_ICON)" -f "$(ICON_FLAGS)" -o "../$(TARGET).smdh"

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval CURBIN := $(patsubst %.shbin.o,%.shbin,$(notdir $@)))
	picasso -o $(CURBIN) $1
	bin2s $(CURBIN) | $(AS) -o $@
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
endef

%.shbin.o : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))
-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
