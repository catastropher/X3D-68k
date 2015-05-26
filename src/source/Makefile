CC=tigcc
C_FILES=$(wildcard *.c)
OBJ_FILES=$(addprefix ,$(notdir $(C_FILES:.c=.o)))
LD_FLAGS=
CC_FLAGS=-I../headers

X3D.a: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CC_FLAGS) -c -o $@ $<
	
install:
	rm -rf "C:\\Program Files (x86)\\TIGCC\\Lib\\X3D.a"
	rm -rf "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	cp -f X3D.a "C:\\Program Files (x86)\\TIGCC\\Lib\\"
	cp -rf ../headers "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	
clean:
	rm -rf *.a
	rm -rf *.o