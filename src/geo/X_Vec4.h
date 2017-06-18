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

#include "math/X_fix.h"
#include "X_Vec3.h"

typedef struct X_Vec4
{
    x_fp16x16 x;
    x_fp16x16 y;
    x_fp16x16 z;
    x_fp16x16 w;
} X_Vec4;

typedef X_Vec4 X_Vec4_fp16x16;

static inline X_Vec4 x_vec4_make(int x, int y, int z, int w)
{
    return (X_Vec4) { x, y, z, w };
}

static inline X_Vec3 x_vec4_to_vec3(const X_Vec4* v)
{
    return x_vec3_make(v->x, v->y, v->z);
}

static inline X_Vec4 x_vec4_add(const X_Vec4* a, const X_Vec4* b)
{
    return x_vec4_make(a->x + b->x, a->y + b->y, a->z + b->z, a->w + b->w);
}

