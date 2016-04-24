#pragma once

#include "X3D_common.h"

typedef struct X3D_PolyVertex {
  X3D_Vex2D v2d;
  int16 intensity;
  int32 u, v;
  int32 z;
} X3D_PolyVertex;

typedef struct X3D_PolyLine {
  uint16 total_v;
  X3D_PolyVertex** v;
} X3D_PolyLine;

typedef struct X3D_PolyLine2 {
  uint16 total_v;
  X3D_PolyVertex* v;
} X3D_PolyLine2;

typedef struct X3D_PolyRegion {
  X3D_PolyLine2 left;
  X3D_PolyLine2 right;
} X3D_PolyRegion;

_Bool x3d_polyline_split(X3D_PolyVertex* v, uint16 total_v, X3D_PolyLine* left, X3D_PolyLine* right);
void x3d_polyline_get_value(X3D_PolyLine* p, int16 y, X3D_PolyVertex* v);


