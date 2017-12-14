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

#include "X_Vec3.h"
#include "math/X_fix.h"

typedef struct X_Plane
{
    X_Vec3 normal;
    x_fp16x16 d;
} X_Plane;

void x_plane_init_from_three_points(X_Plane* plane, const X_Vec3* a, const X_Vec3* b, const X_Vec3* c);
void x_plane_print(const X_Plane* plane);

static inline void x_plane_init_from_normal_and_point_fp16x16(X_Plane* plane, const X_Vec3* normal, const X_Vec3* point)
{
    plane->normal = *normal;
    plane->d = -x_vec3_dot(normal, point);
}

// TODO: needs a better name
static inline x_fp16x16 x_plane_point_distance_fp16x16(const X_Plane* plane, const X_Vec3* point)
{
    return x_vec3_dot(&plane->normal, point) + plane->d;
}

static inline _Bool x_plane_point_is_on_normal_facing_side_fp16x16(const X_Plane* plane, const X_Vec3* point)
{
    return x_plane_point_distance_fp16x16(plane, point) > 0;
}


static inline void x_plane_flip_direction(X_Plane* plane)
{
    plane->normal = x_vec3_neg(&plane->normal);
    plane->d = -plane->d;
}

