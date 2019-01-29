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

#include "math/FixedPoint.hpp"
#include "Vec3.hpp"

template<typename T>
struct Vec4Template
{
    Vec4Template() { }
    constexpr Vec4Template(T x_, T y_, T z_, T w_)
        : x(x_),
        y(y_),
        z(z_),
        w(w_)
    {

    }

    constexpr Vec4Template(const Vec3Template<T>& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = 0;
    }

    constexpr Vec3Template<T> toVec3() const
    {
        return Vec3Template<T>(x, y, z);
    }

    T x;
    T y;
    T z;
    T w;
};

using Vec416x16 = Vec4Template<x_fp16x16>;

typedef struct X_Vec4
{
    x_fp16x16 x;
    x_fp16x16 y;
    x_fp16x16 z;
    x_fp16x16 w;
} X_Vec4;

using Vec4 = Vec4Template<fp>;

typedef X_Vec4 X_Vec4_fp16x16;

static inline X_Vec4 x_vec4_make(int x, int y, int z, int w)
{
    return (X_Vec4) { x, y, z, w };
}

static inline Vec3 x_vec4_to_vec3(const X_Vec4* v)
{
    return Vec3(v->x, v->y, v->z);
}

static inline X_Vec4 x_vec4_from_vec3(X_Vec3_int* v)
{
    return x_vec4_make(v->x, v->y, v->z, 0);
}

static inline X_Vec4 x_vec4_add(const X_Vec4* a, const X_Vec4* b)
{
    return x_vec4_make(a->x + b->x, a->y + b->y, a->z + b->z, a->w + b->w);
}

