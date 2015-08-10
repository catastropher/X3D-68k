.PHONY: all

.PHONY: tools
.PHONY: clean
.PHONY: clean-all
.PHONY: config-all

# 68k
.PHONY: 68k
.PHONY: 68k-ti92plus
.PHONY: 68k-ti89
.PHONY: 68k-v200

.PHONY: config-68k-ti92plus
.PHONY: config-68k-ti89
.PHONY: config-68k-v200

.PHONY: clean-68k
.PHONY: clean-68k-ti92plus
.PHONY: clean-68k-ti89
.PHONY: clean-68k-v200

# pc

.PHONY: pc
.PHONY: config-oc
.PHONY: clean-pc
.PHONY: nuke-build


all: 68k pc

config-all: config-68k config-pc

# 68k

68k: 68k-ti92plus 68k-ti89 68k-v200
	
68k-ti92plus:
	@cd build/68k/ti92plus && make --no-print-directory
	
68k-ti89:
	@cd build/68k/ti89 && make --no-print-directory
	
68k-v200:
	@cd build/68k/v200 && make --no-print-directory
	
clean-68k: clean-68k-ti92plus clean-68k-ti89 clean-68k-v200

clean-68k-ti92plus:
	@cd build/68k/ti92plus && make clean --no-print-directory
	
clean-68k-ti89:
	@cd build/68k/ti89 && make clean --no-print-directory
	
clean-68k-v200:
	@cd build/68k/v200 && make clean --no-print-directory


config-68k: config-68k-ti92plus config-68k-ti89 config-68k-v200
	
config-68k-ti92plus:
	@mkdir -p build/68k/ti92plus && cd build/68k/ti92plus && cmake ../../.. -DX3D_TARGET=68k -DX3D_SUBTARGET=ti92plus -DOUTPUT_DIR=$(X3D)/lib/68k/ti92plus
	
config-68k-ti89:
	@mkdir -p build/68k/ti89 && cd build/68k/ti89 && cmake ../../.. -DX3D_TARGET=68k -DX3D_SUBTARGET=ti89 -DOUTPUT_DIR=$(X3D)/lib/68k/ti89

config-68k-v200:
	@mkdir -p build/68k/v200 && cd build/68k/v200 && cmake ../../.. -DX3D_TARGET=68k -DX3D_SUBTARGET=v200 -DOUTPUT_DIR=$(X3D)/lib/68k/v200
	
	
# pc
pc:
	@cd build/pc && make --no-print-directory
	
config-pc:
	@mkdir -p build/pc && cd build/pc && cmake ../.. -DX3D_TARGET=pc -DOUTPUT_DIR=$(X3D)/lib/pc
	
clean-pc:
	@cd build/pc && make clean --no-print-directory
	
tools:
	@make --no-print-directory -C ./tools
	
clean:
	@echo Run 'make clean-all' to clean all builds.
	
clean-all: clean-68k clean-pc

nuke-build:
	@rm -rf build