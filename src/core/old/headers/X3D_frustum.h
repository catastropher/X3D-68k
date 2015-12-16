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
#include "X3D_polygon.h"

/// A plane described by the equation ax + by + cz - d = 0, where <a, b, c>
/// is the surface normal and d is the perpendicular distance from the origin.
typedef struct X3D_Plane {
  Vex3D_fp0x16 normal;    ///< Surface normal (a vector perpendicular to the plane)
  int16 d;                    ///< Perpendicular distance from the origin
} X3D_Plane;

/// A viewing frustum that can have a variable number of planes. A viewing frustum
/// is a 3D viewing region bounded by planes that defines what a camera can see.
/// Though traditionally a frustum is a truncated pyramid, in X3D a frustum can
/// have any number of planes since it's a portal renderer.
/// @note This is a variable-sized data structure!
typedef struct X3D_Frustum {
  uint16 total_p;     ///< Total number of planes
  X3D_Plane p[0];     ///< Planes (variable number)
} X3D_Frustum;

struct X3D_Frustum;
struct X3D_ViewPort;

void x3d_frustum_print(struct X3D_Frustum* f);
void x3d_frustum_from_rendercontext(X3D_Frustum* f, struct X3D_ViewPort* context);

static inline uint16 x3d_frustum_needed_size(uint16 total_p) {
  return sizeof(X3D_Frustum) + sizeof(X3D_Plane) * total_p;
}

void x3d_construct_frustum_from_polygon3D(X3D_Polygon3D* poly, struct X3D_ViewPort* context, struct X3D_Frustum* dest);

