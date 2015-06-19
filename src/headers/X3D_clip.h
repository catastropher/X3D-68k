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

#pragma once

/// @todo document
// Holds which planes in a frustum a vertex is outside of
// Note: this is a variable sized data structure
typedef struct X3D_VertexClip {
  X3D_Vex3D_int16 v;
  uint16 outside_planes[0];
} X3D_VertexClip;

typedef struct X3D_Edge {
  X3D_Vex3D_int16 a;
  X3D_Vex3D_int16 b;
} X3D_Edge;

