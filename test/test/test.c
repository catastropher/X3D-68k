#define USE_TI92PLUS
#define SAVE_SCREEN
#define EXECUTE_IN_GHOST_SPACE

#include <tigcclib.h>

#include "X3D.h"

void _main() {
  uint16 status;
  
  X3D_Context context;
  X3D_Config config;
  
  config.w = 240;
  
  if((status = x3d_init(&context, &config)) != 0) {
    clrscr();
    printf("Error loading DLL: %d\n", status);
    ngetchx();
  }
}

