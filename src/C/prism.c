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

#include "X3D_common.h"
#include "X3D_matrix.h"
#include "X3D_trig.h"
#include "X3D_vector.h"
#include "X3D_prism.h"
#include "X3D_polygon.h"

/**
* Constructs a 3D prism with regular polygons as the base.
*
* @param s          - pointer to the dest prism
* @param steps      - number of "steps" i.e. points on the polygon base
* @param r          - radius of the base
* @param h          - height of the rism
* @param rot_angle  - angles of rotation around its origin
*
* @return nothing
* @note @ref X3D_Prism is a variable-sized data structure. Make sure s is
*     at least sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * steps * 2 bytes big!
*/
void x3d_prism3d_construct(X3D_Prism3D* s, uint16 steps, uint16 r, int16 h, X3D_Vex3D_angle256 rot_angle) {
  ufp8x8 angle = 0;
  ufp8x8 angle_step = 65536L / steps;
  uint16 i;

  s->base_v = steps;

  // Construct the two bases (regular polygons)
  for(i = 0; i < steps; ++i) {
    s->v[i].x = mul_fp0x16_by_int16_as_int16(x3d_cos(x3d_uint16_upper(angle)), r);
    s->v[i].z = mul_fp0x16_by_int16_as_int16(x3d_sin(x3d_uint16_upper(angle)), r);
    s->v[i].y = -h / 2;

    s->v[i + steps].x = s->v[i].x;
    s->v[i + steps].z = s->v[i].z;
    s->v[i + steps].y = h / 2;

    angle += angle_step;
  }

  // Rotate the prism around its center
  X3D_Mat3x3_fp0x16 mat;
  x3d_mat3x3_construct(&mat, &rot_angle);

  for(i = 0; i < steps * 2; ++i) {
    X3D_Vex3D_int16 rot;

    x3d_vex3d_int16_rotate(&rot, &s->v[i], &mat);
    s->v[i] = rot;
  }
}

/// @todo Document.
void x3d_prism3d_get_face(X3D_Prism3D* prism, uint16 face, X3D_Polygon3D* dest) {
  uint16 i;
  
  if(face <= X3D_BASE_B) {
    // One of the prism bases
    X3D_Vex3D* v;
    int16 dir;
    
    dest->total_v = prism->base_v;
    
    if(face == X3D_BASE_A) {
      v = prism->v;
      dir = 1;
    }
    else {
      v = prism->v + prism->base_v * 2 - 1;
      dir = -1;
    }
    
    uint16 i;
    for(i = 0; i < prism->base_v; ++i) {
      dest->v[i] = *v;
      v += dir;
    }
  }
  else {
    // One of the sides
    
  }
}











