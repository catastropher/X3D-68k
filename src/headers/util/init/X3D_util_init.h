#include "common/X3D_interface.h"

typedef struct X3D_InitSettings {
  int16 screen_w;
  int16 screen_h;
  _Bool fullscreen;
} X3D_InitSettings;

X3D_INTERFACE _Bool x3d_init(X3D_InitSettings* init);