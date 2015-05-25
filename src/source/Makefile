CC=tigcc
CFLAGS=-I../headers -c

all: X3D.a

X3D.a: factorial.o fix.o error.o
	$(CC) factorial.o fix.o error.o -ar -o X3D.a
	
factorial.o: factorial.c
	$(CC) $(CFLAGS) factorial.c

fix.o: fix.c
	$(CC) $(CFLAGS) fix.c
	
error.o: error.c 
	$(CC) $(CFLAGS) error.c
	
install:
	rm -rf "C:\\Program Files (x86)\\TIGCC\\Lib\\X3D.a"
	rm -rf "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	cp -f X3D.a "C:\\Program Files (x86)\\TIGCC\\Lib\\"
	cp -rf ../headers "C:\\Program Files (x86)\\TIGCC\\Include\\C\\X3D"
	
clean:
	rm -rf *.a
	rm -rf *.o