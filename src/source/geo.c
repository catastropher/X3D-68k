// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_geo.h"

/**
* Constructs a plane from 3 points on the plane.
*
* @param p - plane
* @param a - first point
* @param b - middle point
* @param c - end point
*
* @return nothing
* @todo Add tests
*/
void x3d_plane_construct(X3D_Plane* p, X3D_Vex3D_int16* a, X3D_Vex3D_int16* b, X3D_Vex3D_int16* c) {
  // Calculate the normal of the plane
  X3D_Vex3D_int16 v1 = vex3d_int16_sub(a, b);
  X3D_Vex3D_int16 v2 = vex3d_int16_sub(c, b);

  x3d_vex3d_fp0x16_cross(&p->normal, &v1, &v2);

  // D = (AX + BY + CZ)
  p->d = x3d_vex3d_int16_dot(&p->normal, a) >> X3D_NORMAL_SHIFT;
}

/**
* Returns the (signed) distance from a point to a plane. A positive distance
* means the point is on the normal facing side. A positive sign means it's on
* the opposite side. 0 means that the point is directly on the plane.
*
* @param plane    - the plane
* @param v        - point to test
*
* @return The distance to the plane
*/
int16 x3d_distance_to_plane(X3D_Plane* plane, X3D_Vex3D_int16* v) {
  return x3d_vex3d_fp0x16_dot(&plane->normal, v) - plane->d;
}

