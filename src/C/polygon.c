// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>

#include "X3D_common.h"
#include "X3D_polygon.h"

void x3d_polygon3d_print(X3D_Polygon3D* p) {
  printf("X3D_Polygon3D (v = %d)\n", p->total_v);
  
  uint16 i;
  for(i = 0; i < p->total_v; ++i) {
    printf("\t{%d, %d, %d}\n", p->v[i].x, p->v[i].y, p->v[i].z);
  }
  
  printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
/// Translates a 3D polygon.
///
/// @param poly - polygon to translate
/// @param dir  - direction to translate in
/// @param dist - distance to move polygon along dir
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_translate(X3D_Polygon3D* poly, X3D_Normal3D* dir, int16 dist) {
  X3D_Vex3D shift = {
    ((int32)dist * dir->x) >> X3D_NORMAL_BITS,
    ((int32)dist * dir->y) >> X3D_NORMAL_BITS,
    ((int32)dist * dir->z) >> X3D_NORMAL_BITS
  };
  
  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    poly->v[i].x += shift.x;
    poly->v[i].y += shift.y;
    poly->v[i].z += shift.z;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Reverses the points in a 3D polygon. If it was clockwise, it will now be
///   counter-clockwise.
///
/// @param poly - poly to reverse
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_reverse(X3D_Polygon3D* poly) {
  uint16 i;
  
  for(i = 0; i < poly->total_v / 2; ++i) {
    X3D_SWAP(poly->v[i], poly->v[poly->total_v - i - 1]);
  }
}

