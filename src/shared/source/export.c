#define USE_TI89

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')

#include <tigcclib.h>

DLL_INTERFACE

void x3d_init_core();

DLL_ID X3D_ID
DLL_VERSION 1,0
DLL_EXPORTS x3d_init_core

DLL_IMPLEMENTATION

void x3d_init_core() {
  clrscr();
  printf("Hello, world!");
  ngetchx();
}

