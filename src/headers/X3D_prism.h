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
  X3D_Vex3D_int16 v[0];     ///< Vertices (variable number)
} X3D_Prism3D;

typedef X3D_Prism3D X3D_Prism;

/// Face constants for a prism
enum {
  BASE_A = 0,
  BASE_B = 1,
  SIDE_FACE_0 = 2
};

struct X3D_Polygon3D;

//=============================================================================
// Prototypes
//=============================================================================

void x3d_get_prism3d_face(struct X3D_Polygon3D* dest, X3D_Prism3D* prism, uint16 face);
void x3d_set_prism3d_face(struct X3D_Polygon3D* src, X3D_Prism3D* prism, uint16 face);

void x3d_move_polygon3d_along_normal(struct X3D_Polygon3D* p, int16 dist);

