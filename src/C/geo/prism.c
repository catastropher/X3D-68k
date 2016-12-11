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
#include "X3D_render.h"
#include "X3D_enginestate.h"
#include "geo/X3D_line.h"

static inline void x3d_prism3d_construct_top_base(X3D_Prism3D* prism, int16 radius, int16 height);
static inline void x3d_prism3d_construct_bottom_base_from_top(X3D_Prism3D* prism, int16 height);
static inline uint16 x3d_prism3d_calculate_radius_from_side_length(uint16 side_length, uint16 sides_in_base);

void x3d_prism3d_construct(X3D_Prism3D* prism, uint16 sides_in_base, uint16 side_length, int16 height, X3D_Vex3D_angle256 angle) {
    uint16 prism_radius = x3d_prism3d_calculate_radius_from_side_length(side_length, sides_in_base);
    
    x3d_prism3d_set_base_v(prism, sides_in_base);
    x3d_prism3d_construct_top_base(prism, prism_radius, height);
    x3d_prism3d_construct_bottom_base_from_top(prism, height);
    x3d_prism3d_rotate_around_origin(prism, angle);
}

static inline void x3d_prism3d_construct_top_base(X3D_Prism3D* prism, int16 radius, int16 height) {
    ufp8x8 base_angle = 0;
    ufp8x8 angle_step = 65536L / prism->base_v;
    int16 prism_base_y = -height / 2;
    
    for(int i = 0; i < prism->base_v; ++i) {
        X3D_Vex2D circle_point;
        x3d_vex2d_make_point_on_circle(radius, x3d_uint16_upper(base_angle), &circle_point);
        
        prism->v[i] = x3d_vex3d_make(circle_point.x, prism_base_y, circle_point.y);
        base_angle += angle_step;
    }
}

static inline void x3d_prism3d_construct_bottom_base_from_top(X3D_Prism3D* prism, int16 height) {
    for(int i = 0; i < prism->base_v; ++i) {
        prism->v[i + prism->base_v].x = prism->v[i].x;
        prism->v[i + prism->base_v].y = height / 2;
        prism->v[i + prism->base_v].z = prism->v[i].z;
    }
}

static inline uint16 x3d_prism3d_calculate_radius_from_side_length(uint16 side_length, uint16 sides_in_base) {
    return ((int32)side_length << 15) / (2 * x3d_sin(ANG_180 / sides_in_base)); 
}





void x3d_prism3d_rotate_around_origin(X3D_Prism3D* prism, X3D_Vex3D_angle256 angle) {
    X3D_Mat3x3_fp0x16 mat;
    x3d_mat3x3_construct(&mat, &angle);

    for(int i = 0; i < x3d_prism3d_total_vertices(prism); ++i) {
        X3D_Vex3D_int16 rotated;
        x3d_vex3d_int16_rotate(&rotated, &prism->v[i], &mat);
        prism->v[i] = rotated;
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

void x3d_prism3d_get_edge(X3D_Prism3D* prism, uint16 edge, X3D_Ray3D* dest) {
  uint16 a, b;
  x3d_prism_get_edge_index(prism->base_v, edge, &a, &b);
  *dest = x3d_ray3d_make(prism->v[a], prism->v[b]);
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

void x3d_prism3d_center(X3D_Prism3D* prism, X3D_Vex3D* dest) {
  uint16 i;
  X3D_Vex3D_int32 sum = { 0, 0, 0 };
  
  for(i = 0; i < prism->base_v * 2; ++i) {
    sum.x += prism->v[i].x;
    sum.y += prism->v[i].y;
    sum.z += prism->v[i].z;
  }
  
  dest->x = sum.x / (prism->base_v * 2);
  dest->y = sum.y / (prism->base_v * 2);
  dest->z = sum.z / (prism->base_v * 2);
}

void x3d_prism_point_faces(uint16 base_v, uint16 point, uint16* dest) {
  if(point < base_v) {
    dest[0] = X3D_BASE_A;
  }
  else {
    dest[0] = X3D_BASE_B;
    point -= base_v;
  }
 
  dest[1] = point + 2;
  dest[2] = (point != 0 ? point - 1 : base_v - 1) + 2;
  
}

void x3d_prism3d_translate(X3D_Prism3D* prism, X3D_Vex3D* translation) {
  uint16 i;
  for(i = 0; i < prism->base_v * 2; ++i) {
    prism->v[i] = x3d_vex3d_add(prism->v + i, translation);
  }
}

void x3d_prism3d_set_center(X3D_Prism3D* prism, X3D_Vex3D* new_center) {
  X3D_Vex3D center;
  x3d_prism3d_center(prism, &center);
  
  X3D_Vex3D translation = x3d_vex3d_sub(new_center, &center);
  x3d_prism3d_translate(prism, &translation);
}

X3D_Prism3D* x3d_prism3d_construct_temp(uint16 steps, uint16 r, int16 h) {
    static struct {
        X3D_Prism3D prism;
        X3D_Vex3D v[20];
    } temp_prism3d;
    
    x3d_assert(steps <= 10);
    
    temp_prism3d.prism.v = temp_prism3d.v;
    x3d_prism3d_construct(&temp_prism3d.prism, steps, r, h, (X3D_Vex3D_angle256) { 0, 0, 0 });
    
    return &temp_prism3d.prism;
}

