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

all:
	@make --no-print-directory tigcc
	@make --no-print-directory gcc

tigcc:
	@echo Building X3D for tigcc
	@mkdir -p tigcc
	@make --no-print-directory tigcc/X3D.a

gcc:
	@echo Building X3D for gcc
	@mkdir -p gcc
	@make --no-print-directory gcc/X3D.a
	
gcc/X3D.a: $(GCC_OBJ_FILES)
	@ar rcs $(LD_FLAGS) -o gcc/X3D.a $^
	@echo X3D built successfully for gcc
	
gcc/%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CC_FLAGS) -c -o $@ $<
	
tigcc/X3D.a: $(TIGCC_OBJ_FILES)
	@ar rcs $(LD_FLAGS) -o tigcc/X3D.a $^
	@echo X3D built successfully for tigcc
	@echo
	
tigcc/%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CC_FLAGS) -c -o $@ $< -D__TIGCC_HEADERS__
	
install:
	@rm -rf "C:\\Program Files (x86)\\TIGCC\\Lib\\X3D.a"
	@rm -rf "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	@echo Copying X3D.a to TIGCC lib directory...
	@cp -f ./tigcc/X3D.a "C:\\Program Files (x86)\\TIGCC\\Lib\\"
	@echo Copying header files to TIGCC include file directory... 
	@cp -rf ../headers "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	@echo X3D installed.
	
clean:
	@rm -rf *.a
	@rm -rf *.o
	@rm -rf gcc
	@rm -rf tigcc
	@echo Project cleaned.