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

struct X_Plane;

void x_plane_init_from_three_points(X_Plane* plane, const Vec3* a, const Vec3* b, const Vec3* c);

struct X_Plane
{
    X_Plane() { }

    X_Plane(const Vec3fp& a, const Vec3fp& b, const Vec3fp& c)
    {
        Vec3 va = MakeVec3(a);
        Vec3 vb = MakeVec3(b);
        Vec3 vc = MakeVec3(c);

        x_plane_init_from_three_points(this, &va, &vb, &vc);
    }

    X_Plane(const Vec3fp& normal_, const Vec3fp& point)
    {
        normal = normal_;
        d = -normal.dot(point);
    }

    Vec3fp normal;
    fp d;
};

struct X_Mat4x4;
struct X_CameraObject;


void x_plane_print(const X_Plane* plane);
void x_plane_get_orientation(X_Plane* plane, struct X_CameraObject* cam, struct X_Mat4x4* dest);

static inline void x_plane_init_from_normal_and_point(X_Plane* plane, const Vec3* normal, const Vec3* point)
{
    plane->normal = MakeVec3fp(*normal);
    plane->d = -x_vec3_dot(normal, point);
}

// TODO: needs a better name
static inline x_fp16x16 x_plane_point_distance(const X_Plane* plane, const Vec3* point)
{
    Vec3fp temp = MakeVec3fp(*point);

    return (plane->normal.dot(temp) + plane->d).toFp16x16();
}

static inline bool x_plane_point_is_on_normal_facing_side(const X_Plane* plane, const Vec3* point)
{
    return x_plane_point_distance(plane, point) > 0;
}


static inline void x_plane_flip_direction(X_Plane* plane)
{
    plane->normal = -plane->normal;
    plane->d = -plane->d;
}

