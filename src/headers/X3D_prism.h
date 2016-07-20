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

#include <string.h>

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_assert.h"
#include "X3D_polygon.h"
#include "X3D_screen.h"

enum {
  X3D_BASE_A = 0,   /// First base of a prism
  X3D_BASE_B = 1    /// Second base of a prism
};

///////////////////////////////////////////////////////////////////////////////
/// A 3D shape that has two connected bases, each with the same number of
///   vertices (for example, an octagonal prism). The vertices can be anything
///   so long as the 3D shape is convex.
///
/// @note This is a variable-sized data structure!
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Prism3D {
  uint16 base_v;      ///< Number of vertices in one of the bases
  X3D_Vex3D* v;      ///< The base_v * 2 vertices that compose the prism. The
                      ///   first base_v vertices are for X3D_BASE_A and the
                      ///   next base_v vertices are for X3D_BASE_B.
} X3D_Prism3D;

/// @todo Document.
typedef struct X3D_Prism2D {
  uint16 base_v;
  X3D_Vex2D v[];
} X3D_Prism2D;


///////////////////////////////////////////////////////////////////////////////
/// Returns the size of a 3D prism (in bytes) given the number of vertices in
///   the base.
///
/// @param base_v - number of vertices in the base
///
/// @return The size of the prism.
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_prism3d_size(uint16 base_v) {
  return 2 * base_v * sizeof(X3D_Vex3D);
}

///////////////////////////////////////////////////////////////////////////////
/// Copies a 3D prism.
///
/// @param dest - dest prism
/// @param src  - src prism
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
static inline void x3d_prism3d_copy(X3D_Prism3D* dest, X3D_Prism3D* src) {
  memcpy(dest->v, src->v, x3d_prism3d_size(src->base_v));
  dest->base_v = src->base_v;
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the total number of faces in a 3D prism given the number of
///   vertices in the base.
///
/// @param base_v - number of vertices in the base.
///
/// @return The number of faces such a 3D prism would have.
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_prism3d_total_f(uint16 base_v) {
  return base_v + 2;
}

#define X3D_ALLOCA_PRISM3D(_base_v) (X3D_Prism3D) { .v = alloca(_base_v * 2 * sizeof(X3D_Point3D)), .base_v = _base_v }

struct X3D_DisplayLineList;
struct X3D_CameraObject;
struct X3D_RasterRegion;
struct X3D_Ray3D;

void x3d_prism3d_construct(X3D_Prism3D* s, uint16 steps, uint16 r, int16 h, X3D_Vex3D_angle256 rot_angle);
void x3d_prism3d_get_face(X3D_Prism3D* prism, uint16 face, X3D_Polygon3D* dest);
void x3d_prism3d_set_face(X3D_Prism3D* prism, uint16 face, X3D_Polygon3D* src);
uint16 x3d_prism_face_edge_indexes(uint16 base_v, uint16 face, uint16* dest);
void x3d_prism_get_edge_index(uint16 base_v, uint16 edge, uint16* a, uint16* b);
void x3d_prism_get_edge_pairs(uint16 base_v, X3D_Pair* dest);
void x3d_prism3d_center(X3D_Prism3D* prism, X3D_Vex3D* dest);
void x3d_prism_point_faces(uint16 base_v, uint16 point, uint16* dest);

void x3d_prism3d_get_edge(X3D_Prism3D* prism, uint16 edge, struct X3D_Ray3D* dest);
void x3d_prism3d_translate(X3D_Prism3D* prism, X3D_Vex3D* translation);

