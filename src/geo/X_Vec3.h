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

////////////////////////////////////////////////////////////////////////////////
/// A 3D vector or vertex.
////////////////////////////////////////////////////////////////////////////////
typedef struct X_Vec3
{
    int x;
    int y;
    int z;
} X_Vec3;

/// A 3D vertex or vector with fp16x16 components.
typedef X_Vec3 X_Vec3_fp16x16;

////////////////////////////////////////////////////////////////////////////////
/// Initializes a 3D vector with the given coordinates.
////////////////////////////////////////////////////////////////////////////////
static inline void x_vec3_init(X_Vec3* v, int x, int y, int z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

////////////////////////////////////////////////////////////////////////////////
/// Returns a 3D vector with the given coordinates.
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_make(int x, int y, int z)
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
static inline X_Vec3 x_vec3_scale(const X_Vec3* v, int scale)
{
    return x_vec3_make
    (
        v->x * scale,
        v->y * scale,
        v->z * scale
    );
}

////////////////////////////////////////////////////////////////////////////////
/// Divides a 3D vector by the integer @param divisor.
/// @return v / divisor
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_div(const X_Vec3* v, int divisor)
{
    return x_vec3_make
    (
        v->x / divisor,
        v->y / divisor,
        v->z / divisor
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
static inline _Bool x_vec3_equal(const X_Vec3* a, const X_Vec3* b)
{
    return a->x == b->y && a->y == b->y && a->z == b->z;
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the dot product of two 3D vectors. Mathematically, a dot b =
///     |a||b|cos theta.
/// @return a dot b
////////////////////////////////////////////////////////////////////////////////
static inline int x_vec3_dot(const X_Vec3* a, const X_Vec3* b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the cross product of two 3D vectors, which is a vector that is
///     orthogonal (perpendicular) to both a and b.
///
/// @return a cross b
/// @note (a cross b) == -(b cross a)
/// @note This does not normalize the resulting vector.
////////////////////////////////////////////////////////////////////////////////
static inline X_Vec3 x_vec3_cross(const X_Vec3* a, const X_Vec3* b)
{
    return x_vec3_make(
        a->y * b->z - b->y * a->z,
        a->z * b->x - b->z * a->x,
        a->x * b->y - b->x * a->y
    );
}

////////////////////////////////////////////////////////////////////////////////
/// Determines whether two 3D vectors are orthogonal (perpendicular) to each
///     other.
///
/// @return a is perpendicular to b
////////////////////////////////////////////////////////////////////////////////
static inline _Bool x_vec3_is_orthogonal_to(const X_Vec3* a, const X_Vec3* b)
{
    return x_vec3_dot(a, b) == 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the length of a vector squared.
/// @return length(v)^2
////////////////////////////////////////////////////////////////////////////////
static inline int x_vec3_length_squared(const X_Vec3* v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the distance squared between two points.
/// @return (distance between a and b)^2
////////////////////////////////////////////////////////////////////////////////
static inline int x_vec3_distance_squared(const X_Vec3* a, const X_Vec3* b)
{
    X_Vec3 diff = x_vec3_sub(a, b);
    return x_vec3_length_squared(&diff);
}

int x_vec3_length(const X_Vec3* v);
int x_vec3_distance(const X_Vec3* a, const X_Vec3* b);

