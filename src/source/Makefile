ifeq ($(MAKECMDGOALS),gcc/X3D.a)
CC=gcc
else
ifeq ($(MAKECMDGOALS),tigcc/X3D.a)
CC=tigcc
endif
endif

C_FILES=$(wildcard *.c)
GCC_OBJ_FILES=$(addprefix gcc/,$(notdir $(C_FILES:.c=.o)))
TIGCC_OBJ_FILES=$(addprefix tigcc/,$(notdir $(C_FILES:.c=.o)))
LD_FLAGS=
CC_FLAGS=-I../headers

.PHONY: tigcc
.PHONY: gcc
.PHONY: all
.PHONE: test


all:
	@make --no-print-directory tigcc
	@make --no-print-directory gcc

tigcc:
	@echo Building X3D for tigcc
	@mkdir -p tigcc
	@make --no-print-directory tigcc/X3D.a

	@rm -rf ../../test/tigcc

test:
	@echo ""
	@make --no-print-directory install
	@echo ""
	@make --no-print-directory -C ../../test/

gcc:
	@echo Building X3D for gcc
	@mkdir -p gcc
	@make --no-print-directory gcc/X3D.a

	@rm -rf ../../test/gcc
	
gcc/X3D.a: $(GCC_OBJ_FILES)
	@ar rcs $(LD_FLAGS) -o gcc/X3D.a $^
	@echo X3D built successfully for gcc
	
gcc/%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CC_FLAGS) -c -o $@ $<
	
tigcc/X3D.a: $(TIGCC_OBJ_FILES)
	@$(CC) $(LD_FLAGS) -o $@ $^ -ar
	@echo X3D built successfully for tigcc
	@echo
	
tigcc/%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CC_FLAGS) -c -o $@ $< -D__TIGCC_HEADERS__ -O2 -fomit-frame-pointer -mno-bss -Wa,--all-relocs -mregparm=5 -Wa,-l --optimize-code --cut-ranges --reorder-sections --merge-constants -ffunction-sections -fdata-sections -fmerge-all-constants -mpcrel
	
install: check-tigcc
	@echo Installing X3D for tigcc...
	@rm -rf "$(TIGCC)/Include/C/X3D"
	@cp -f ./tigcc/X3D.a "$(TIGCC)/lib"
	@cp -rf ../headers "$(TIGCC)/Include/C/X3D"

	@echo Installing X3D for gcc
	@rm -rf /usr/include/X3D
	@cp -rf ../headers /usr/include/X3D
	@cp gcc/X3D.a /usr/lib/libX3D.a

	@echo X3D installed.
	
check-tigcc:
ifndef TIGCC
	$(error TIGCC is not set to TIGCC directory)
endif
	
clean:
	@rm -rf *.a
	@rm -rf *.o
	@rm -rf gcc
	@rm -rf tigcc
	@echo Project cleaned.