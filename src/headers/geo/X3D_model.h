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

#include "X3D_prism.h"

typedef struct X3D_Range_int16 {
  int16 min, max;
} X3D_Range_int16;

typedef struct X3D_AABB {
  X3D_Range_int16 x;
  X3D_Range_int16 y;
  X3D_Range_int16 z;
} X3D_AABB;


typedef struct X3D_BoundSphere {
  X3D_Vex3D center;
  int16 radius;
} X3D_BoundSphere;

typedef struct X3D_Model {
    X3D_Vex3D* v;
    uint16 total_v;
    
    uint16* triangle_vertices;
    uint16 total_triangles;
} X3D_Model;

