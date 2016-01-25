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
#include "X3D_vector.h"
#include "X3D_plane.h"
#include "X3D_assert.h"
#include "X3D_matrix.h"

#pragma once

typedef struct X3D_Polygon3D {
  uint16 total_v;
  X3D_Vex3D* v;
} X3D_Polygon3D;

typedef struct X3D_Polygon2D {
  uint16 total_v;
  X3D_Vex2D* v;
} X3D_Polygon2D;

void x3d_polygon3d_print(X3D_Polygon3D* p);
void x3d_polygon3d_translate(X3D_Polygon3D* poly, X3D_Normal3D* dir, int16 dist);
void x3d_polygon3d_reverse(X3D_Polygon3D* poly);
void x3d_polygon2d_to_polygon3d(X3D_Polygon2D* poly, X3D_Polygon3D* dest, X3D_Plane* plane, X3D_Vex3D* top_left, X3D_Vex3D* bottom_right, X3D_Mat3x3* mat);
void x3d_polygon2d_construct(X3D_Polygon2D* poly, uint16 steps, int16 r, angle256 ang);

static inline uint16 x3d_polygon3d_size(uint16 total_v) {
  return sizeof(X3D_Polygon3D) + total_v * sizeof(X3D_Vex3D);
}

static inline void x3d_polygon3d_calculate_plane(X3D_Polygon3D* poly, X3D_Plane* plane) {
  x3d_assert(poly->total_v >= 3);
  x3d_plane_construct(plane, poly->v, poly->v + 1, poly->v + 2);
}

