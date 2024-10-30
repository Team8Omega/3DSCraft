
TOOLS 	 := tools

ifeq ($(OS),Windows_NT)
	WGET := $(TOOLS)/wget.exe
	OS	 := win
else
	WGET := wget
	OS	 := ubuntu
endif

MAKEROM_URL := https://api.github.com/repos/3DSGuy/Project_CTR/releases/latest

MAKEROM 	:= $(shell curl -s $(MAKEROM_URL) | grep -o 'http[^"]*\.zip' | grep $(OS))

.PHONY: makerom

makerom:
	@echo Download makerom...
	@$(WGET) -q -P $(TOOLS) $(MAKEROM)
	@unzip -q -o $(TOOLS)/makerom*.zip -d $(TOOLS)
	@rm -f $(TOOLS)/makerom*.zip*
	@if [ "$(OS)" = "ubuntu" ]; then \
		chmod +x tools/makerom*; \
	fi

bannertool:
	@echo Building bannertool...
	@rm -rf 3ds-bannertool
	@git clone https://github.com/carstene1ns/3ds-bannertool
	@cd 3ds-bannertool && cmake -DCMAKE_C_COMPILER=/usr/bin/cc -DCMAKE_CXX_COMPILER=/usr/bin/c++ . && make
	@cp 3ds-bannertool/bannertool* ../$(TOOLS)
	@rm -rf 3ds-bannertool