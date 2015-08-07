#define USE_TI89

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')

#include <tigcclib.h>

DLL_INTERFACE

void test();

DLL_ID X3D_ID
DLL_VERSION 1,0
DLL_EXPORTS test

DLL_IMPLEMENTATION

void test() {
  printf("Hello, world!");
}

