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

////////////////////////////////////////////////////////////////////////////////
/// A 3D vector or vertex.
////////////////////////////////////////////////////////////////////////////////
// typedef struct X_Vec3_int
// {
//     int x;
//     int y;
//     int z;
// } X_Vec3_int;

typedef struct X_Vec3
{
    x_fp16x16 x;
    x_fp16x16 y;
    x_fp16x16 z;
} X_Vec3;

typedef X_Vec3 X_Vec3_int;

/// A 3D vertex or vector with fp16x16 components.

typedef X_Vec3_int X_Vec3_fp0x30;

typedef struct X_Vec3_float
{
    float x;
    float y;
    float z;
} X_Vec3_float;

typedef struct X_Vec3_short
{
    short x;
    short y;
    short z;
} X_Vec3_short;

void x_vec3_normalize(X_Vec3* v);
void x_vec3_fp16x16_print(const X_Vec3* v, const char* label);

////////////////////////////////////////////////////////////////////////////////
/// Returns a 3D vector with the given coordinates.
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_make(x_fp16x16 x, x_fp16x16 y, x_fp16x16 z)
{
    return (X_Vec3) { x, y, z };
}

////////////////////////////////////////////////////////////////////////////////
/// Returns the 3D origin.
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_origin(void)
{
    return x_vec3_make(0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
/// Adds two 3D vectors.
/// @return a + b
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_add(const X_Vec3* a, const X_Vec3* b)
{
    return x_vec3_make(a->x + b->x, a->y + b->y, a->z + b->z);
}

static inline X_Vec3 x_vec3_add_scaled(const X_Vec3* a, const X_Vec3* vecToScale, x_fp16x16 scale)
{
    return x_vec3_make
    (
        a->x + x_fp16x16_mul(vecToScale->x, scale),
        a->y + x_fp16x16_mul(vecToScale->y, scale),
        a->z + x_fp16x16_mul(vecToScale->z, scale)
    );
}

static inline X_Vec3 x_vec3_fp16x16_scale(const X_Vec3* v, x_fp16x16 scale)
{
    return x_vec3_make
    (
        x_fp16x16_mul(v->x, scale),
        x_fp16x16_mul(v->y, scale),
        x_fp16x16_mul(v->z, scale)
    );
}

static inline X_Vec3 x_vec3_shift_right(X_Vec3* v, int shift)
{
    return x_vec3_make(v->x >> shift, v->y >> shift, v->z >> shift);
}

////////////////////////////////////////////////////////////////////////////////
/// Adds three 3D vectors.
/// @return a + b + c
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_add_three(const X_Vec3* a, const X_Vec3* b, const X_Vec3* c)
{
    return x_vec3_make
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
static inline X_Vec3 x_vec3_sub(const X_Vec3* a, const X_Vec3* b)
{
    return x_vec3_make(a->x - b->x, a->y - b->y, a->z - b->z);
}

////////////////////////////////////////////////////////////////////////////////
/// Scales a 3D vector by the integer @param scale.
/// @return v * scale
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_scale_int(const X_Vec3* v, int scale)
{
    return x_vec3_make
    (
        v->x * scale,
        v->y * scale,
        v->z * scale
    );
}

static inline X_Vec3 x_vec3_scale(const X_Vec3* v, x_fp16x16 scale)
{
    return x_vec3_make
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
static inline X_Vec3 x_vec3_neg(const X_Vec3* v)
{
    return x_vec3_make(-v->x, -v->y, -v->z);
}

////////////////////////////////////////////////////////////////////////////////
/// Determines whether two 3D vectors are equal.
/// @return a == b
////////////////////////////////////////////////////////////////////////////////
static inline bool x_vec3_equal(const X_Vec3* a, const X_Vec3* b)
{
    return a->x == b->y && a->y == b->y && a->z == b->z;
}

static inline x_fp16x16 x_vec3_dot(const X_Vec3* a, const X_Vec3* b)
{
    return x_fp16x16_mul(a->x, b->x) + x_fp16x16_mul(a->y, b->y) + x_fp16x16_mul(a->z, b->z);
}


static inline X_Vec3 x_vec3_cross(const X_Vec3* a, const X_Vec3* b)
{
    return x_vec3_make(
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
static inline bool x_vec3_is_orthogonal_to(const X_Vec3* a, const X_Vec3* b)
{
    return x_vec3_dot(a, b) == 0;
}


static inline x_fp16x16 x_vec3_length(const X_Vec3* v)
{
    return x_sqrt(x_fp16x16_mul(v->x, v->x) + x_fp16x16_mul(v->y, v->y) + x_fp16x16_mul(v->z, v->z)) << 8;
}


static inline X_Vec3_float x_vec3_float_make(float x, float y, float z)
{
    return (X_Vec3_float) { x, y, z };
}

static inline X_Vec3_int x_vec3_to_vec3_int(const X_Vec3* src)
{
    return (X_Vec3_int) { src->x >> 16, src->y >> 16, src->z >> 16 };
}

static inline X_Vec3 x_vec3_int_to_vec3(const X_Vec3_int* src)
{
    return x_vec3_make(src->x << 16, src->y << 16, src->z << 16);
}

static inline X_Vec3 x_vec3_float_to_vec3_int(const X_Vec3_float* v)
{
    return (X_Vec3_int) { (int)v->x, (int)v->y, (int)v->z };
}

static inline X_Vec3 x_vec3_float_to_vec3(const X_Vec3_float* v)
{
    return x_vec3_make(v->x * 65536, v->y * 65536, v->z * 65536);
}

static inline X_Vec3 x_vec3_convert_quake_coord_to_x3d_coord(const X_Vec3* v)
{
    return x_vec3_make(v->y, -v->z, -v->x);
}


