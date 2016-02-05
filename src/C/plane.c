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
#include "X3D_vector.h"
#include "X3D_plane.h"

/**
* Constructs a plane from 3 points on the plane.
*
* @param p - plane
* @param a - first point
* @param b - middle point
* @param c - end point
*
* @return nothing
* @todo Fix formatting.
*/
void x3d_plane_construct(X3D_Plane* p, X3D_Vex3D_int16* a, X3D_Vex3D_int16* b, X3D_Vex3D_int16* c) {
  // Calculate the normal of the plane
  X3D_Vex3D v1 = x3d_vex3d_sub(a, b);
  X3D_Vex3D v2 = x3d_vex3d_sub(c, b);

  x3d_vex3d_fp0x16_cross(&p->normal, &v1, &v2);
  
#ifdef __68k__
  p->normal = x3d_vex3d_neg(&p->normal);
#endif

  // D = (AX + BY + CZ)
  p->d = x3d_vex3d_int16_dot(&p->normal, a) >> X3D_NORMAL_BITS;
}

void x3d_plane_print(X3D_Plane* p) {
  printf("X3D_Plane\n\tNormal: {%d, %d, %d}\n\tD: %d\n",
    p->normal.x, p->normal.y, p->normal.z, p->d);
}

