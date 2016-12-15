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

typedef struct X3D_Line3D {
  X3D_Vex3D start;
  X3D_Vex3D dir;
} X3D_Line3D;

typedef struct X3D_Ray3D {
  X3D_Vex3D v[2];
} X3D_Ray3D;

typedef struct X3D_Ray2D {
  X3D_Vex2D v[2];
} X3D_Ray2D;

struct X3D_Polygon3D;
struct X3D_CameraObject;
struct X3D_Plane;

typedef enum {
  X3D_RAY3D_NOT_CLIPPED = 0,
  X3D_RAY3D_V0_CLIPPED = 1,
  X3D_RAY3D_V1_CLIPPED = 2,
  X3D_RAY3D_INVISIBLE = X3D_RAY3D_V0_CLIPPED | X3D_RAY3D_V1_CLIPPED
} X3D_Ray3DClipStatus;

_Bool x3d_line3d_intersect_plane(X3D_Line3D* line, struct X3D_Plane* plane, X3D_Vex3D* inter, int16* scale);
_Bool x3d_line3d_intersect_polygon(X3D_Line3D* line, struct X3D_Polygon3D* poly, X3D_Vex3D* inter, int16* scale);
void x3d_line3d_from_screen_point(X3D_Line3D* line, X3D_Vex2D* p, struct X3D_CameraObject* cam, int16 near_z);

X3D_Ray3DClipStatus x3d_ray3d_clip_to_near_plane(X3D_Ray3D* ray, X3D_Ray3D* dest);
void x3d_ray3d_interpolate(X3D_Ray3D* ray, fp0x16 t, X3D_Vex3D* dest);
void x3d_ray3d_interpolate_fp8x8(X3D_Ray3D* ray, fp8x8 t, X3D_Vex3D* dest);
_Bool x3d_ray3d_partially_in_front_of_near_plane(X3D_Ray3D* ray);
_Bool x3d_ray3d_totally_in_front_of_near_plane(X3D_Ray3D* ray);
void x3d_ray3d_interpolate(X3D_Ray3D* ray, fp0x16 t, X3D_Vex3D* dest);

static inline void x3d_ray3d_set_v(X3D_Ray3D* ray, int16 v_id, X3D_Vex3D* src) {
    ray->v[v_id] = *src;
}

static inline void x3d_ray3d_swap_v(X3D_Ray3D* ray) {
  X3D_SWAP(ray->v[0], ray->v[1]);
}

static inline X3D_Ray3D x3d_ray3d_make(X3D_Vex3D v1, X3D_Vex3D v2) {
  return (X3D_Ray3D) {
      { v1, v2 }
  };
}

static inline void x3d_ray3d_midpoint(X3D_Ray3D* ray, X3D_Vex3D* dest) {
  dest->x = ((int32)ray->v[0].x + ray->v[1].x) / 2;
  dest->y = ((int32)ray->v[0].y + ray->v[1].y) / 2;
  dest->z = ((int32)ray->v[0].z + ray->v[1].z) / 2;
}

void x3d_ray3d_project_to_ray2d(X3D_Ray3D* ray, X3D_Ray2D* dest);


