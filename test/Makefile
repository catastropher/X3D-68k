ifeq ($(MAKECMDGOALS),gcc/test.9xz)
CC=gcc
else
ifeq ($(MAKECMDGOALS),tigcc/test.9xz)
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
	@echo ""
	@make --no-print-directory gcc

	@echo ""
	@echo "=======Running tests for gcc======="
	@gcc/test

tigcc:
	@echo Building X3D test suite for tigcc
	@mkdir -p tigcc
	@make --no-print-directory tigcc/test.9xz

gcc:
	@echo Building X3D test suite for gcc
	@mkdir -p gcc
	@make --no-print-directory gcc/test.exe

tigcc/test.9xz: $(TIGCC_OBJ_FILES)
	@$(CC) -o "test" $(TIGCC_OBJ_FILES) X3D.a
	@mv test.9xz tigcc/test.9xz
	@echo X3D test suite built successfully for tigcc

tigcc/%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CC_FLAGS) -c -o $@ $< -D__TIGCC_HEADERS__

gcc/test.exe: $(GCC_OBJ_FILES)
	@$(CC) -o "gcc/test" $(GCC_OBJ_FILES) -lX3D
	@echo X3D test suite built successfully for gcc

gcc/%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CC_FLAGS) -c -o $@ $< -O3

clean:
	@rm -rf gcc
	@rm -rf tigcc