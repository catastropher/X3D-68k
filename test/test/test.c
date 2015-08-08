#define USE_TI92PLUS
#define SAVE_SCREEN
#define EXECUTE_IN_GHOST_SPACE

#include <tigcclib.h>

#include "X3D.h"

void _main() {
  uint16 status;
  
  if((status = x3d_init()) != 0) {
    clrscr();
    printf("Error loading DLL: %d\n", status);
    ngetchx();
  }
}

