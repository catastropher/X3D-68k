.PHONY: tools
.PHONY: 68k
.PHONY: pc

68k:
	@cd build/68k && make --no-print-directory
	
pc:
	@cd build/pc && make --no-print-directory
	
clean-68k:
	@cd build/68k && make clean --no-print-directory

config-68k:
	
	
	
config-68k-ti92plus:
	@mkdir -p build/68k && cd build/68k && cmake ../.. -DX3D_TARGET=68k -DX3D_SUBTARGET=ti92plus -DOUTPUT_DIR=$(X3D)/lib/68k/ti92plus
	
	
config-pc:
	@mkdir -p build/pc && cd build/pc && cmake ../.. -DX3D_TARGET=pc
	
tools:
	@make --no-print-directory -C ./tools