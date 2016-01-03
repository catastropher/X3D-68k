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
  X3D_Vex3D v[];      ///< The base_v * 2 vertices that compose the prism. The
                      ///   first base_v vertices are for X3D_BASE_A and the
                      ///   next base_v vertices are for X3D_BASE_B.
} X3D_Prism3D;


///////////////////////////////////////////////////////////////////////////////
/// Returns the size of a 3D prism (in bytes) given the number of vertices in
///   the base.
///
/// @param base_v - number of vertices in the base
///
/// @return The size of the prism.
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_prism3d_size(uint16 base_v) {
  return sizeof(X3D_Prism3D) + base_v * sizeof(X3D_Vex3D);
}

