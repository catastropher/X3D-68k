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

#pragma once

#include "X3D_fix.h"
#include "X3D_vector.h"

//=============================================================================
// Types
//=============================================================================

/// A prism i.e. a variable-sized 3D shape with two bases that have the same
/// number of points.
/// @note Despite the name, an X3D_Prism doesn't have to have congruent,
/// parallel bases like the mathematical definition. The only constraint
/// is that the prism must remain convex for it to work properly with the rest
/// of the engine and both bases have to have the same number of points.
/// @note This is a variable-sized data structure!
typedef struct X3D_Prism3D {
  uint32 draw_edges;        ///< Bitfield of which edges should be drawn
  uint16 base_v;            ///< Number of vertices in each base
  Vex3D v[0];     ///< Vertices (variable number)
} X3D_Prism3D;

typedef X3D_Prism3D X3D_Prism;

/// Face constants for a prism
enum {
  BASE_A = 0,
  BASE_B = 1,
  SIDE_FACE_0 = 2
};

struct X3D_Polygon3D;

static inline uint16 x3d_opposite_vertex(X3D_Prism3D* prism, uint16 v) {
  return prism->base_v + (prism->base_v - v) - 1;
}

static inline void x3d_prism3d_side_face(X3D_Prism3D* prism, uint16 face, uint16 v[4]) {
  v[0] = face - 2;
  v[1] = x3d_opposite_vertex(prism, face - 2);

  v[2] = x3d_opposite_vertex(prism, x3d_single_wrap(v[0] + 1, prism->base_v));

  v[3] = x3d_single_wrap(v[0] + 1, prism->base_v);
}

//=============================================================================
// Prototypes
//=============================================================================

void x3d_prism3d_get_face(struct X3D_Polygon3D* dest, X3D_Prism3D* prism, uint16 face);
void x3d_prism3d_set_face(struct X3D_Polygon3D* src, X3D_Prism3D* prism, uint16 face);

void x3d_move_polygon3d_along_normal(struct X3D_Polygon3D* p, int16 dist, Vex3D* center);
void x3d_prism3d_get_center(X3D_Prism3D* prism, Vex3D* dest);

