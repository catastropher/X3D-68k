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

#include "math/X_sqrt.h"
#include "math/X_fix.h"
#include "math/X_convert.hpp"

////////////////////////////////////////////////////////////////////////////////
/// A 3D vector or vertex.
////////////////////////////////////////////////////////////////////////////////


template<typename T>
struct Vec3Template
{
    Vec3Template(T x_, T y_, T z_) : x(x_), y(y_), z(z_) { }
    Vec3Template() : x(0), y(0), z(0) { }
    
    Vec3Template operator+(const Vec3Template& v) const
    {
        return Vec3Template(x + v.x, y + v.y, z + v.z);
    }
    
    Vec3Template operator-(const Vec3Template& v) const
    {
        return Vec3Template(x - v.x, y - v.y, z - v.z);
    }
    
    Vec3Template operator+=(const Vec3Template& v)
    {
        *this = *this + v;
        return *this;
    }

    Vec3Template toX3dCoords() const
    {
        return Vec3Template(y, -z, -x);
    }

    T dot(const Vec3Template& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec3Template operator-() const
    {
        return Vec3Template(-x, -y, -z);
    }
    
    T x;
    T y;
    T z;
};

using Vec3fp = Vec3Template<fp>;

template<typename T, typename U>
inline Vec3Template<T> operator*(const Vec3Template<T>& a, const U& b)
{
    return Vec3Template<T>(a.x * b, a.y * b, a.z * b);
}

template<typename From, typename To>
inline void convert(Vec3Template<From>& from, Vec3Template<To>& to)
{
    convert(from.x, to.x);
    convert(from.y, to.y);
    convert(from.z, to.z);
}

// FIXME: need specialization becuase Vec3 can't use fp yet
template<>
inline void convert(Vec3Template<float>& from, Vec3Template<x_fp16x16>& to)
{
    to.x = convert<fp>(from.x).toFp16x16();
    to.y = convert<fp>(from.y).toFp16x16();
    to.z = convert<fp>(from.z).toFp16x16();
}

using Vec3 = Vec3Template<x_fp16x16>;

// For the refactoring effort
static inline Vec3 MakeVec3(const Vec3fp v)
{
    return Vec3(v.x.internalValue(), v.y.internalValue(), v.z.internalValue());
}

// For the refactoring effort
static inline Vec3fp MakeVec3fp(const Vec3 v)
{
    return Vec3fp(fp(v.x), fp(v.y), fp(v.z));
}


typedef Vec3 X_Vec3_int;

/// A 3D vertex or vector with fp16x16 components.

typedef X_Vec3_int X_Vec3_fp0x30;

using X_Vec3_float = Vec3Template<float>;
using Vec3f = Vec3Template<float>;

typedef struct X_Vec3_short
{
    short x;
    short y;
    short z;
} X_Vec3_short;

void x_vec3_normalize(Vec3* v);
void x_vec3_fp16x16_print(const Vec3* v, const char* label);


////////////////////////////////////////////////////////////////////////////////
/// Returns the 3D origin.
////////////////////////////////////////////////////////////////////////////////
static inline Vec3 x_vec3_origin(void)
{
    return Vec3();
}

static inline Vec3 x_vec3_add_scaled(const Vec3* a, const Vec3* vecToScale, x_fp16x16 scale)
{
    return Vec3
    (
        a->x + x_fp16x16_mul(vecToScale->x, scale),
        a->y + x_fp16x16_mul(vecToScale->y, scale),
        a->z + x_fp16x16_mul(vecToScale->z, scale)
    );
}

static inline Vec3 x_vec3_fp16x16_scale(const Vec3* v, x_fp16x16 scale)
{
    return Vec3
    (
        x_fp16x16_mul(v->x, scale),
        x_fp16x16_mul(v->y, scale),
        x_fp16x16_mul(v->z, scale)
    );
}

static inline Vec3 x_vec3_shift_right(Vec3* v, int shift)
{
    return Vec3(v->x >> shift, v->y >> shift, v->z >> shift);
}

////////////////////////////////////////////////////////////////////////////////
/// Adds three 3D vectors.
/// @return a + b + c
////////////////////////////////////////////////////////////////////////////////
static inline Vec3 x_vec3_add_three(const Vec3* a, const Vec3* b, const Vec3* c)
{
    return Vec3
    (
        a->x + b->x + c->x,
        a->y + b->y + c->y,
        a->z + b->z + c->z
    );
}

////////////////////////////////////////////////////////////////////////////////
/// Subtracts two 3D vectors.
/// @return a - b
////////////////////////////////////////////////////////////////////////////////
static inline Vec3 x_vec3_sub(const Vec3* a, const Vec3* b)
{
    return Vec3(a->x - b->x, a->y - b->y, a->z - b->z);
}

////////////////////////////////////////////////////////////////////////////////
/// Scales a 3D vector by the integer @param scale.
/// @return v * scale
////////////////////////////////////////////////////////////////////////////////
static inline Vec3 x_vec3_scale_int(const Vec3* v, int scale)
{
    return Vec3
    (
        v->x * scale,
        v->y * scale,
        v->z * scale
    );
}

static inline Vec3 x_vec3_scale(const Vec3* v, x_fp16x16 scale)
{
    return Vec3
    (
        x_fp16x16_mul(v->x, scale),
        x_fp16x16_mul(v->y, scale),
        x_fp16x16_mul(v->z, scale)
    );
}

////////////////////////////////////////////////////////////////////////////////
/// Negates a 3D vector (flips its direction).
/// @return -v
////////////////////////////////////////////////////////////////////////////////
static inline Vec3 x_vec3_neg(const Vec3* v)
{
    return Vec3(-v->x, -v->y, -v->z);
}

////////////////////////////////////////////////////////////////////////////////
/// Determines whether two 3D vectors are equal.
/// @return a == b
////////////////////////////////////////////////////////////////////////////////
static inline bool x_vec3_equal(const Vec3* a, const Vec3* b)
{
    return a->x == b->y && a->y == b->y && a->z == b->z;
}

static inline x_fp16x16 x_vec3_dot(const Vec3* a, const Vec3* b)
{
    return x_fp16x16_mul(a->x, b->x) + x_fp16x16_mul(a->y, b->y) + x_fp16x16_mul(a->z, b->z);
}


static inline Vec3 x_vec3_cross(const Vec3* a, const Vec3* b)
{
    return Vec3(
        x_fp16x16_mul(a->y, b->z) - x_fp16x16_mul(b->y, a->z),
        x_fp16x16_mul(a->z, b->x) - x_fp16x16_mul(b->z, a->x),
        x_fp16x16_mul(a->x, b->y) - x_fp16x16_mul(b->x, a->y)
    );
}

////////////////////////////////////////////////////////////////////////////////
/// Determines whether two 3D vectors are orthogonal (perpendicular) to each
///     other.
///
/// @return a is perpendicular to b
////////////////////////////////////////////////////////////////////////////////
static inline bool x_vec3_is_orthogonal_to(const Vec3* a, const Vec3* b)
{
    return x_vec3_dot(a, b) == 0;
}


static inline x_fp16x16 x_vec3_length(const Vec3* v)
{
    return x_sqrt(x_fp16x16_mul(v->x, v->x) + x_fp16x16_mul(v->y, v->y) + x_fp16x16_mul(v->z, v->z)) << 8;
}


static inline X_Vec3_float x_vec3_float_make(float x, float y, float z)
{
    return (X_Vec3_float) { x, y, z };
}

static inline X_Vec3_int x_vec3_to_vec3_int(const Vec3* src)
{
    return (X_Vec3_int) { src->x >> 16, src->y >> 16, src->z >> 16 };
}

static inline Vec3 x_vec3_int_to_vec3(const X_Vec3_int* src)
{
    return Vec3(src->x << 16, src->y << 16, src->z << 16);
}

static inline Vec3 x_vec3_float_to_vec3_int(const X_Vec3_float* v)
{
    return (X_Vec3_int) { (int)v->x, (int)v->y, (int)v->z };
}

static inline Vec3 x_vec3_float_to_vec3(const X_Vec3_float* v)
{
    return Vec3(v->x * 65536, v->y * 65536, v->z * 65536);
}

static inline Vec3 x_vec3_convert_quake_coord_to_x3d_coord(const Vec3* v)
{
    return Vec3(v->y, -v->z, -v->x);
}


