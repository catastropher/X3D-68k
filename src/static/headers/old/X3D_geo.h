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
#include "X3D_prism.h"
#include "X3D_polygon.h"
#include "X3D_frustum.h"

//=============================================================================
// Defines
//=============================================================================

#define ALLOCA_POLYGON3D(_total_v) alloca(x3d_polygon3d_needed_size(_total_v))
#define ALLOCA_FRUSTUM(_total_p) alloca(x3d_frustum_needed_size(_total_p))



//=============================================================================
// Types
//=============================================================================




//=============================================================================
// Function prototypes
//=============================================================================

void x3d_plane_construct(X3D_Plane* p, X3D_Vex3D_int16* a, X3D_Vex3D_int16* b, X3D_Vex3D_int16* c);
int16 x3d_distance_to_plane(X3D_Plane* plane, X3D_Vex3D_int16* v);


//=============================================================================
// Static inline functions
//=============================================================================

// Returns the size needed to store a Polygon3D with the given number of points
static inline uint16 x3d_polygon3d_needed_size(uint16 total_v) {
  return sizeof(X3D_Polygon3D) + sizeof(X3D_Vex3D_int16) * total_v;
}
