CC=tigcc
C_FILES=$(wildcard *.c)
OBJ_FILES=$(addprefix ,$(notdir $(C_FILES:.c=.o)))
LD_FLAGS=
CC_FLAGS=-I../headers

X3D.a: $(OBJ_FILES)
	@echo Linking...
	@$(CC) $(LD_FLAGS) -o $@ $^ -ar
	@echo X3D built successfully.

%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CC_FLAGS) -c -o $@ $<
	
install:
	@rm -rf "C:\\Program Files (x86)\\TIGCC\\Lib\\X3D.a"
	@rm -rf "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	@echo Copying X3D.a to TIGCC lib directory...
	@cp -f X3D.a "C:\\Program Files (x86)\\TIGCC\\Lib\\"
	@echo Copying header files to TIGCC include file directory... 
	@cp -rf ../headers "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	@echo X3D installed.
	
clean:
	@rm -rf *.a
	@rm -rf *.o
	@echo Project cleaned.