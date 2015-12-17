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
#include "X3D_vector.h"
#include "X3D_polygon.h"

/**
* Finds the geometric center of a 3D polygon.
* 
* @param poly     - polygon
* @param center   - where to store the center point
*
* @return nothing
*
* @note If the polygon is convex, the center point is guaranteed to be inside
*     the polygon. If concave, it may be, but this is not guaranteed.
*/
void x3d_polygon3d_center(X3D_Polygon3D* poly, X3D_Vex3D_int16* center) {
  Vex3D_int32 sum = { 0, 0, 0 };
  uint16 i;
  
  for(i = 0; i < poly->total_v; ++i) {
    sum.x += poly->v[i].x;
    sum.y += poly->v[i].y;
    sum.z += poly->v[i].z;
  }
  
  center->x = sum.x / poly->total_v;
  center->y = sum.y / poly->total_v;
  center->z = sum.z / poly->total_v;
}

/**
* Translates a 3D polygon by a vector.
* 
* @param poly     - polygon
* @param v        - vector to translate by
*
* @return nothing
*/
void x3d_polygon3d_translate(X3D_Polygon3D* poly, X3D_Vex3D_int16* v) {
  uint16 i;
  
  for(i = 0; i < poly->total_v; ++i) {
    poly->v[i] = vex3d_int16_add(&poly->v[i], v);
  }
}


/**
* Scales a polygon, relative to its center.
*
* @param poly     - polygon
* @param scale    - scaling factor between 0 and 1 (as an fp8x8)
*
* @return nothing
*/
void x3d_polygon3d_scale(X3D_Polygon3D* poly, fp8x8 scale) {
  X3D_Vex3D_int16 center, ncenter;
  uint16 i;

  // Move the polygon to its relative center
  x3d_polygon3d_center(poly, &center);
  ncenter = vneg16(&center);
  x3d_polygon3d_translate(poly, &ncenter);

  // Scale each vertex
  for(i = 0; i < poly->total_v; ++i) {
    poly->v[i] = vscale16(poly->v + i, scale);
  }

  // Move it back to its absolute center
  x3d_polygon3d_translate(poly, &center);
}
