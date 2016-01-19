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
#include "X3D_camera.h"

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
    
    printf("%d %d\n", s->v[i].x, s->v[i].z);

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
    dest->total_v = 4;
    
    uint16 f = face - 2;
    uint16 next = (f + 1 < prism->base_v ? f + 1 : 0);
    
    dest->v[0] = prism->v[f];
    dest->v[3] = prism->v[next];
    dest->v[2] = prism->v[next + prism->base_v];
    dest->v[1] = prism->v[f + prism->base_v];
  }
}

/// @todo Document.
void x3d_prism3d_set_face(X3D_Prism3D* prism, uint16 face, X3D_Polygon3D* src) {
  if(face <= X3D_BASE_B) {
    // One of the prism bases
    X3D_Vex3D* v;
    int16 dir;
    
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
      *v = src->v[i];
      v += dir;
    }
  }
  else {
    // One of the sides
    src->total_v = 4;
    
    uint16 f = face - 2;
    uint16 next = (f + 1 < prism->base_v ? f + 1 : 0);
    
    prism->v[f] = src->v[0];
    prism->v[next] = src->v[3];
    prism->v[next + prism->base_v] = src->v[2];
    prism->v[f + prism->base_v] = src->v[1];
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Gets a list of edges indexes that compose a prism face. For example,
///   given an octahedron prism, X3D_BASE_A would be composed of edges 0 - 7.
///
/// @param base_v - number of vertices in the prism base
/// @param face   - face ID
/// @param dest   - where to write the indexes
///
/// @return The number of edges in the face.
/// @note dest should be big enough to hold the number of edges in the
///   requested face. If face is X3D_BASE_A or X3D_BASE_B, this will be base_v
///   vertices. Otherwise, it will be 4.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_prism_face_edge_indexes(uint16 base_v, uint16 face, uint16* dest) {
  if(face <= X3D_BASE_B) {
    // Prism base
    uint16 start = (face == X3D_BASE_A ? 0 : base_v);
    uint16 i;
    
    for(i = 0; i < base_v; ++i)
      dest[i] = start + i;
    
    return base_v;
  }
  else {
    // Quad face
    uint16 f = face - 2;
    dest[0] = f;
    dest[1] = x3d_int16_add_wrap(f, 1, base_v) + base_v * 2;
    dest[2] = f + base_v;
    dest[3] = f + base_v * 2;
    
    return 4;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the vertex indexes that compose a 2D/3D prism edge.
///
/// @param prism  - prism
/// @param edge   - edge ID (0 to base_v * 3)
/// @param a      - dest of first vertex
/// @param b      - dest of second vertex
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_prism_get_edge_index(uint16 base_v, uint16 edge, uint16* a, uint16* b) {
  if(edge < base_v * 2) {
    uint16 offset = (edge < base_v ? 0 : base_v);
    uint16 next = (edge - offset + 1 != base_v ? edge - offset + 1 : 0)
      + offset;
    
    *a = edge;
    *b = next;
  }
  else {
    *a = edge - base_v * 2;
    *b = edge - base_v;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Gets a list of edge pairs for prism with the given number of vertices in
///   the base. For a given edge, this gives the index of the two vertices
///   that compose the edge.
///
/// @param base_v - number of vertices in the base
/// @param dest   - array of X3D_Pair big enough to hold base_v * 3 pairs
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_prism_get_edge_pairs(uint16 base_v, X3D_Pair* dest) {
  uint16 i;
  
  for(i = 0; i < base_v * 3; ++i) {
    x3d_prism_get_edge_index(base_v, i, dest[i].val, dest[i].val + 1);
  }
}

