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
#include "X3D_plane.h"
#include "geo/X3D_line.h"
#include "level/X3D_level.h"

struct X3D_Polygon3D;
struct X3D_CameraObject;
struct X3D_Level;

typedef struct X3D_RayTracer {
  struct X3D_CameraObject* cam;
  X3D_Line3D ray;
  struct X3D_Level* level;
  
  X3D_LEVEL_SEG hit_seg;
  uint16 hit_seg_face;
  int16 min_hit_scale;
  X3D_Vex3D hit_pos;
} X3D_RayTracer;

void x3d_raytracer_init_from_point_on_screen(X3D_RayTracer* raytracer, struct X3D_Level* level, struct X3D_CameraObject* cam, X3D_Vex2D point_on_screen);
_Bool x3d_raytracer_find_closest_hit_wall(X3D_RayTracer* raytracer);

