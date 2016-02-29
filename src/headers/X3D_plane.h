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

#pragma once

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_matrix.h"

// AX + BY + CZ - D = 0
typedef struct X3D_Plane {
  X3D_Normal3D normal;
  int16 d;
} X3D_Plane;

typedef struct X3D_Frustum {
  uint16 total_p;
  X3D_Plane* p;
} X3D_Frustum;

void x3d_plane_construct(X3D_Plane* p, X3D_Vex3D_int16* a, X3D_Vex3D_int16* b, X3D_Vex3D_int16* c);
void x3d_plane_print(X3D_Plane* p);
_Bool x3d_plane_guess_orientation(X3D_Plane* plane, X3D_Mat3x3* dest, X3D_Vex3D* p);

///////////////////////////////////////////////////////////////////////////////
/// Calculates the distance from a point to a plane.
///
/// @param p  - plane
/// @param v  - point
///
/// @return The distance from p to v.
///////////////////////////////////////////////////////////////////////////////
static inline int16 x3d_plane_dist(X3D_Plane* p, X3D_Vex3D* v) {
  return ((int16)x3d_vex3d_fp0x16_dot(&p->normal, v) - p->d);
}
