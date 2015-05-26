#define SAVE_SCREEN

#include <tigcclib.h>

#include <X3D/X3D.h>

int _ti92plus;

void _main() {
	x3d_errorif(5 > 4, "WRONG! %d %s", 7, "Hello");
}