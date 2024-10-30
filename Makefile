#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(CURDIR)),)
CURDIR := $(shell pwd)
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#-------------------------------------------------------------

ifeq ($(DEBUG),0)
	DEBUG		:=	0
else ifeq ($(DEB),0)
	DEBUG		:=	0
else ifeq ($(D),0)
	DEBUG		:=	0
else ifeq ($(RELEASE),1)
	DEBUG		:=	0
else ifeq ($(REL),1)
	DEBUG		:=	0
else ifeq ($(R),1)
	DEBUG		:=	0
else
	DEBUG		:=  1
endif

ifeq ($(DEBUG),0)
	TAG_YES 	:= release
	TAG_NO 		:= debug
else
	TAG_YES 	:= debug
	TAG_NO 		:= release
endif

#--------------------------------------------------------------

TARGET 			:=  3DSCraft# Project Name

ifeq ($(WORKFLOW),1)
  ifeq ($(DEBUG),1)
    TARGET 		:= $(TARGET)-debug
  endif
endif

#--------------------------------------------------------------

# Project
BUILD			:=	build
DATA			:=	data
META			:=	project
ROMFS			:=	romfs
INCLUDES		:=	lib include .
SRCDIRS 		:= 	assets source

# Version
VERSION_MAJOR	:= 0
VERSION_MINOR	:= 5
VERSION_MICRO	:= 4

# 3dsx
APP_DESCRIPTION :=  "3DSCraft - Craftus Gen. 4" # HB Channel Description
APP_AUTHOR		:=  "Team-Omega" # HB Channel Author
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

LDFLAGS	=	-specs=3dsx.specs -z noexecstack -g $(ARCH) -Wl,-Map,$(notdir $*.map) -include $(DEVKITPRO)/libctru/include/3ds/types.h

CFLAGS	:=	-g -Wall -Wno-psabi -mword-relocations \
			-DC_V=\"$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)\" \
			-ffunction-sections -ffast-math\
			$(ARCH) $(LDFLAGS)

CFLAGS	+=	$(INCLUDE) -D__3DS__ -D_3DS=1 -D_VER_MAJ=$(VERSION_MAJOR) -D_VER_MIN=$(VERSION_MINOR) -D_VER_MIC=$(VERSION_MICRO) -D_AUTHOR=$(APP_AUTHOR) -D_GNU_SOURCE=1
CFLAGS  +=  `$(PREFIX)pkg-config opusfile --cflags`

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS	:=	-g $(ARCH)

ifeq ($(DEBUG), 0)
CFLAGS		+=	-fomit-frame-pointer -O2
LIBS		:= -lcitro3d -lctru 
else
CFLAGS		+=	-Og -fsanitize=undefined -fsanitize-trap -D_DEBUG
LIBS		:= -lcitro3dd -lctrud
endif

LIBS		+= -lm `$(PREFIX)pkg-config opusfile --libs` -lgame

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

SOURCES 		:= 	$(foreach dir,$(SRCDIRS),$(patsubst $(CURDIR)/%,%, $(shell find $(realpath $(dir)) -type d)))

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
# tools
#---------------------------------------------------------------------------------------

ifneq ($(OS),Windows_NT)
MAKEROM      ?= tools/makerom
else
MAKEROM      ?= tools/makerom.exe
endif

MAKEROM_ARGS += -elf "$(TARGET).elf" -rsf "$(RSF_PATH)" -banner "$(BUILD)/banner.bnr" -icon "$(TARGET).smdh"
MAKEROM_ARGS += -major $(VERSION_MAJOR) -minor $(VERSION_MINOR) -micro $(VERSION_MICRO) -desc app:4

ifneq ($(strip $(LOGO)),)
	MAKEROM_ARGS += -logo "$(LOGO)"
endif

#---------------------------------------------------------------------------------
# building start
#---------------------------------------------------------------------------------

.PHONY: $(BUILD) clean all makerom_check bannertool_check greet init

init: greet
	@mkdir -p $(BUILD)

	@if [ ! -e "$(BUILD)/$(TAG_YES)" ] || [ -e "$(BUILD)/$(TAG_NO)" ]; then \
		echo "tag" > "$(BUILD)/$(TAG_YES)"; \
		rm -f "$(BUILD)/$(TAG_NO)"; \
		if [ "$(wildcard $(BUILD)/*)" ]; then \
			echo "# Detected change of target!"; \
			$(MAKE) --no-print-directory -f $(CURDIR)/Makefile clean; \
		fi; \
		$(MAKE) --no-print-directory -f $(CURDIR)/Makefile clean-lib; \
	fi

	@$(MAKE) --no-print-directory -f $(CURDIR)/Makefile lib $(BUILD);

greet:
	@echo -n "# Compiling $(TARGET), v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO) - "
ifneq ($(strip $(DEBUG)),0)
	@echo "Debug"
else
	@echo "Release"
endif

all: init cia

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

makerom_check:
	@if [ ! -f $(MAKEROM) ]; then \
		make  --no-print-directory -f tools/prepare.mak makerom; \
	else \
		echo "# Makerom"; \
	fi

#---------------------------------------------------------------------------------
# build installable cia file
#---------------------------------------------------------------------------------

cxi:
	@$(MAKEROM) -o $(TARGET).cxi $(MAKEROM_ARGS)
	@echo "built ... $(TARGET).cxi (Code)"

cfa:
	@$(MAKEROM) -o $(TARGET).cfa -rsf $(RSF_PATH) -target t
	@echo "built ... $(TARGET).cfa (RomFS)"

cia: makerom_check clean-cia cfa cxi 
	@$(MAKEROM) -f cia -o $(TARGET).cia -target t -i $(TARGET).cxi:0:0 -i $(TARGET).cfa:1:1
	@echo "built ... $(TARGET).cia (Final Package)"

#---------------------------------------------------------------------------------------
# library
#---------------------------------------------------------------------------------------

ifneq ($(OS),Windows_NT)
AR 		:= $(DEVKITARM)/bin/arm-none-eabi-ar
else
AR 		:= $(DEVKITARM)/bin/arm-none-eabi-ar.exe
endif

LIBSOURCES 	:= $(wildcard lib/**/*.c)
LIBOBJS 	:= $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(LIBSOURCES)))

lib: lib/libgame.a

lib/libgame.a: $(LIBOBJS)
	@echo Building library file...
	@$(AR) rcs lib/libgame.a $^

lib/%.o: lib/%.c
	@echo $@...
	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

#---------------------------------------------------------------------------------
# ctru compile
#---------------------------------------------------------------------------------

ctru:
	@echo Compiling libctru, make sure it exists at ./private/libctru/
	@make -C private/libctru -f ./Makefile --no-print-directory
	@cp private/libctru/lib/* ./lib/
	@echo Copied to lib/libctru*.a

#---------------------------------------------------------------------------------
# clean
#---------------------------------------------------------------------------------

clean: clean-exe
	@echo "# Cleaning build"
	@rm -rf $(BUILD) 

clean-exe: clean-cia
ifneq ($(WORKFLOW),1)
	@echo "# Cleaning targets"
	@rm -f $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf $(TARGET).lst $(BUILD)/banner.bnr
endif

clean-cia:
	@echo "# Cleaning packages"
	@rm -f $(TARGET).cia $(TARGET).cxi $(TARGET).cfa

clean-lib:
	@echo "# Cleaning libraries"
	@rm -f lib/**/*.o
	@rm -f lib/libgame.o

clean-tool:
	@echo "# Cleaning tools"
	@rm -f tools/makerom* tools/bannertool*

clean-all: clean clean-lib clean-tool
	@echo "# Cleaned everything"

#---------------------------------------------------------------------------------
# debugging
#---------------------------------------------------------------------------------

run:
	@echo running...
	@3dslink $(TARGET).3dsx
	
rund: #run dima
	@3dslink $(TARGET).3dsx -a 192.168.178.37

emud: init
	@citra-qt $(TARGET).3dsx

emue: init
	@../../Desktop/citra-windows-msvc-20240303-0ff3440/citra-qt.exe $(TARGET).3dsx

ADDR2LINE   ?= $(DEVKITARM)/bin/arm-none-eabi-addr2line*

addr:
	@$(ADDR2LINE) -e $(TARGET).elf -i -r -p -f -s $(A)


#---------------------------------------------------------------------------------
#
# inside build/ directory
#
#---------------------------------------------------------------------------------

else

DEPENDS	:=	$(OFILES:.o=.d)

ifneq ($(OS),Windows_NT)
BANNERTOOL   ?= ../tools/bannertool
else
BANNERTOOL   ?= ../tools/bannertool.exe
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
$(OUTPUT).3dsx	:	bannertool_check $(OUTPUT).elf $(OUTPUT).smdh banner.bnr
else
$(OUTPUT).3dsx	:	bannertool_check $(OUTPUT).elf banner.bnr
endif

3dsx: $(OUTPUT).3dsx

$(OUTPUT).elf	:	$(OFILES)

banner.bnr:
	@$(BANNERTOOL) makebanner $(BANNER_IMAGE_ARG) "../$(BANNER_IMAGE)" $(BANNER_AUDIO_ARG) "../$(BANNER_AUDIO)" -o "banner.bnr"

$(OUTPUT).smdh:
	@$(BANNERTOOL) makesmdh -s "$(TARGET)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -i "$(APP_ICON)" -f "$(ICON_FLAGS)" -o "../$(TARGET).smdh"

bannertool_check:
	@if [ ! -f $(BANNERTOOL) ]; then \
		make --no-print-directory  -f ../tools/prepare.mak bannertool; \
	else \
		echo "# Bannertool"; \
	fi

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
