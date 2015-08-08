#define USE_TI92PLUS

#include <tigcclib.h>

#include "X3D.h"

void _main() {
  if(!x3d_init()) {
    clrscr();
    printf("Error loading DLL\n");
    ngetchx();
  }
  
  ngetchx();
}

