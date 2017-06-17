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

typedef struct X_Ray3
{
    X_Vec3 v[2];
} X_Ray3;

static inline X_Ray3 x_ray3_make(X_Vec3 start, X_Vec3 end)
{
    return (X_Ray3)
    {
        { start, end }
    };
}

static inline void x_ray3d_lerp(const X_Ray3* ray, x_fp16x16 t, X_Vec3* dest)
{
    dest->x = x_lerp(ray->v[0].x, ray->v[1].x, t);
    dest->y = x_lerp(ray->v[0].y, ray->v[1].y, t);
    dest->z = x_lerp(ray->v[0].z, ray->v[1].z, t);
}

