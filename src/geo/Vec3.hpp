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

#include <cmath>

#include "math/FastSqrt.hpp"
#include "math/FixedPoint.hpp"
#include "math/Convert.hpp"

////////////////////////////////////////////////////////////////////////////////
/// A 3D vector or vertex.
////////////////////////////////////////////////////////////////////////////////


template<typename T>
struct Vec3Template
{
    constexpr Vec3Template(T x_, T y_, T z_) : x(x_), y(y_), z(z_) { }
    constexpr Vec3Template() : x(0), y(0), z(0) { }
    
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
    
    Vec3Template cross(const Vec3Template& v) const
    {
        return Vec3Template(
            y * v.z - v.y * z,
            z * v.x - v.z * x,
            x * v.y - v.x * y);
    }

    Vec3Template scale(const Vec3Template& v) const
    {
        return Vec3Template(
            x * v.x,
            y * v.y,
            z * v.z);
    }

    bool operator==(const Vec3Template& v) const
    {
        return x == v.x && y == v.y && z == v.z;
    }

    bool operator!=(const Vec3Template& v) const
    {
        return !(*this == v);
    }

    void print(const char* name) const
    {
        printf("%s: %f %f %f\n", name, convert<float>(x), convert<float>(y), convert<float>(z));
    }

    void normalize();

    // FIXME: don't use floats
    T length()
    {
        float xx = convert<float>(x);
        float yy = convert<float>(y);
        float zz = convert<float>(z);

        return convert<T>(sqrtf(xx * xx + yy * yy + zz * zz));
    }

    template<typename U>
    Vec3Template<U> toVec3()
    {
        return Vec3Template<U>(
            convert<U>(x),
            convert<U>(y),
            convert<U>(z));
    }
    
    T x;
    T y;
    T z;
};

using Vec3fp = Vec3Template<fp>;
using Vec3i = Vec3Template<int>;

template<typename T, typename U>
inline Vec3Template<T> operator*(const Vec3Template<T>& a, const U& b)
{
    return Vec3Template<T>(a.x * b, a.y * b, a.z * b);
}

template<typename T, typename U>
inline Vec3Template<T> operator/(const Vec3Template<T>& a, const U& b)
{
    return Vec3Template<T>(a.x / b, a.y / b, a.z / b);
}

template<typename From, typename To>
inline void convert(Vec3Template<From>& from, Vec3Template<To>& to)
{
    convert(from.x, to.x);
    convert(from.y, to.y);
    convert(from.z, to.z);
}

// FIXME: need specialization because Vec3 can't use fp yet
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

static inline X_Vec3_int x_vec3_to_vec3_int(const Vec3* src)
{
    return (X_Vec3_int) { src->x >> 16, src->y >> 16, src->z >> 16 };
}

static inline Vec3 x_vec3_int_to_vec3(const X_Vec3_int* src)
{
    return Vec3(src->x << 16, src->y << 16, src->z << 16);
}

static inline Vec3 x_vec3_float_to_vec3(const X_Vec3_float* v)
{
    return Vec3(v->x * 65536, v->y * 65536, v->z * 65536);
}

static inline Vec3 x_vec3_convert_quake_coord_to_x3d_coord(const Vec3* v)
{
    return Vec3(v->y, -v->z, -v->x);
}

template<>
inline void Vec3fp::normalize()
{
    Vec3 v = MakeVec3(*this);
    x_vec3_normalize(&v);
    *this = MakeVec3fp(v);
}

