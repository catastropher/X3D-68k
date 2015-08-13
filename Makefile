.PHONY: all

.PHONY: tools
.PHONY: clean
.PHONY: clean-all
.PHONY: config-all

# 68k
.PHONY: 68k
.PHONY: ti92plus
.PHONY: ti89
.PHONY: v200

.PHONY: config-ti92plus
.PHONY: config-ti89
.PHONY: config-v200

.PHONY: clean-68k
.PHONY: clean-ti92plus
.PHONY: clean-ti89
.PHONY: clean-v200

# pc

.PHONY: pc
.PHONY: config-oc
.PHONY: clean-pc
.PHONY: nuke-build

.PHONY: test


all: 68k pc

config-all: config-68k config-pc

# 68k

68k: ti92plus ti89 v200
	
ti92plus:
	@cd build/68k/ti92plus && make --no-print-directory
	
ti89:
	@cd build/68k/ti89 && make --no-print-directory
	
v200:
	@cd build/68k/v200 && make --no-print-directory
	
clean-68k: clean-ti92plus clean-ti89 clean-v200

clean-ti92plus:
	@cd build/68k/ti92plus && make clean --no-print-directory
	
clean-ti89:
	@cd build/68k/ti89 && make clean --no-print-directory
	
clean-v200:
	@cd build/68k/v200 && make clean --no-print-directory


config-68k: config-ti92plus config-ti89 config-v200
	
config-ti92plus:
	@mkdir -p build/68k/ti92plus && cd build/68k/ti92plus && cmake ../../.. -DX3D_TARGET=68k -DX3D_SUBTARGET=ti92plus -DOUTPUT_DIR=$(X3D)/lib/68k/ti92plus -DPLATFORM_DIR=$(X3D)/src/platform/68k
	
config-ti89:
	@mkdir -p build/68k/ti89 && cd build/68k/ti89 && cmake ../../.. -DX3D_TARGET=68k -DX3D_SUBTARGET=ti89 -DOUTPUT_DIR=$(X3D)/lib/68k/ti89 -DPLATFORM_DIR=$(X3D)/src/platform/68k

config-v200:
	@mkdir -p build/68k/v200 && cd build/68k/v200 && cmake ../../.. -DX3D_TARGET=68k -DX3D_SUBTARGET=v200 -DOUTPUT_DIR=$(X3D)/lib/68k/v200 -DPLATFORM_DIR=$(X3D)/src/platform/68k
	
	
# pc
pc:
	@cd build/pc && make --no-print-directory
	
config-pc:
	@mkdir -p build/pc && cd build/pc && cmake ../.. -DX3D_TARGET=pc -DOUTPUT_DIR=$(X3D)/lib/pc -DPLATFORM_DIR=$(X3D)/src/platform/pc
	
clean-pc:
	@cd build/pc && make clean --no-print-directory
	
tools:
	@make --no-print-directory -C ./tools
	
clean:
	@echo Run 'make clean-all' to clean all builds.
	
clean-all: clean-68k clean-pc

nuke-build:
	@rm -rf build
	
config-test:
	@mkdir -p build/test && cd build/test && cmake ../../test -DOUTPUT_DIR=$(X3D)/lib/68k/ti92plus -DPLATFORM_DIR=$(X3D)/src/platform/68k
	
test:
	@cd build/test && make --no-print-directory