#include "X3D_config.h"

#ifdef __TIGCC__

#define X3D_ID (((unsigned long)'X' << 16) | ((unsigned long)'3' << 8) | 'D')

DLL_INTERFACE

void x3d_init_core();

DLL_ID X3D_ID
DLL_VERSION 1,0
DLL_EXPORTS x3d_init_core

DLL_IMPLEMENTATION


#endif