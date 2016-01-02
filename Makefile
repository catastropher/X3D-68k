.PHONY: all
.PHONY: test
.PHONY: test-manual
.PHONY: x3d
.PHONY: config

all: test

config:
	@tput setaf 2
	@echo Configuring X3D
	@tput setaf 7
	@mkdir -p build/X3D
	@cd build/X3D && cmake ../../src
	
	@tput setaf 2
	@echo Configuring Unit Tests
	@tput setaf 7
	@mkdir -p build/unit
	@cd build/unit && cmake ../../test/unit
	
	@tput setaf 2
	@echo Configuring Manual Tests
	@tput setaf 7
	@mkdir -p build/manual
	@cd build/manual && cmake ../../test/pc/test-pc
	
x3d:
	@cd build/X3D && make --no-print-directory
	
test: x3d
	@cd build/unit && make --no-print-directory && ./unit
	
test-manual:
	@cd build/manual && make --no-print-directory && ./test-pc