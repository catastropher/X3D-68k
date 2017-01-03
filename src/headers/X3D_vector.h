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

/// The number of bits in an X3D normal (as in surface normal)
#define X3D_NORMAL_BITS 15

///////////////////////////////////////////////////////////////////////////////
/// A 3D vector with int16 components.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Vex3D_int16 {
  int16 x;
  int16 y;
  int16 z;
} X3D_Vex3D_int16;


typedef struct X3D_Vex3D_float {
    float x;
    float y;
    float z;
} X3D_Vex3D_float;

///////////////////////////////////////////////////////////////////////////////
/// A 3D vector with uint8 components.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Vex3D_uint8 {
  uint8 x;
  uint8 y;
  uint8 z;
} X3D_Vex3D_uint8;

///////////////////////////////////////////////////////////////////////////////
/// A 3D vector with int32 components.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Vex3D_int32 {
  int32 x;
  int32 y;
  int32 z;
} X3D_Vex3D_int32;

///////////////////////////////////////////////////////////////////////////////
/// A 2D vector with int16 components.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Vex2D_int16 {
  int32 x;
  int32 y;
} X3D_Vex2D_int16;

typedef struct X3D_Vex2D_int32 {
  int32 x;
  int32 y;
} X3D_Vex2D_int32;

typedef X3D_Vex3D_int32 X3D_Vex3D_fp16x16;

typedef X3D_Vex3D_int16 X3D_Vex3D;
typedef X3D_Vex3D_int16 X3D_Vex3D_fp0x16;
typedef X3D_Vex3D_int16 X3D_Vex3D_fp8x8;

typedef X3D_Vex2D_int16 X3D_Vex2D;

typedef X3D_Vex3D_uint8 X3D_Vex3D_angle256;

typedef X3D_Vex3D_int32 X3D_Vex3D_fp16x8;

struct X3D_Mat3x3_fp0x16;

typedef struct X3D_Point2D_int8 {
  int8 x;
  int8 y;
} X3D_Point2D_int8;

void x3d_vex3d_int16_project(X3D_Vex2D_int16* dest, const X3D_Vex3D_int16* src);
void x3d_vex3d_int16_rotate(X3D_Vex3D_int16* dest, X3D_Vex3D_int16* src, struct X3D_Mat3x3_fp0x16* mat);
void x3d_vex3d_fp0x16_cross(X3D_Vex3D_fp0x16* dest, X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b);
int32 x3d_vex3d_dot(X3D_Vex3D* a, X3D_Vex3D* b);
fp0x16 x3d_vex3d_fp0x16_dot(X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b);
void x3d_vex3d_fp0x16_normalize(X3D_Vex3D_fp0x16* v);
uint16 x3d_vex3d_int16_mag(X3D_Vex3D_int16* v);

void x3d_vex2d_rotate_around_point(X3D_Vex2D* v, X3D_Vex2D* point_to_rotate_around, angle256 angle, X3D_Vex2D* dest);
void x3d_vex2d_make_point_on_circle(int16 radius, angle256 angle, X3D_Vex2D* dest);


static inline X3D_Vex2D x3d_vex2d_make(int x, int y) {
    return (X3D_Vex2D) { x, y };
}

static inline X3D_Vex2D x3d_vex2d_origin() {
    return x3d_vex2d_make(0, 0);
}


typedef X3D_Vex3D_fp0x16 X3D_Normal3D;

static inline X3D_Vex3D x3d_vex3d_sub(X3D_Vex3D* a, X3D_Vex3D* b) {
  return (X3D_Vex3D) {
    a->x - b->x,
    a->y - b->y,
    a->z - b->z
  };
}

static inline X3D_Vex3D x3d_vex3d_add(X3D_Vex3D* a, X3D_Vex3D* b) {
  return (X3D_Vex3D) {
    a->x + b->x,
    a->y + b->y,
    a->z + b->z
  };
}

static inline X3D_Vex3D x3d_vex3d_neg(X3D_Vex3D* v) {
  return (X3D_Vex3D) { -v->x, -v->y, -v->z };
}

static inline _Bool x3d_vex3d_equal(X3D_Vex3D* a, X3D_Vex3D* b) {
  return a->x == b->x && a->y == b->y && a->z == b->z;
}

static inline _Bool x3d_vex2d_equal(X3D_Vex2D* a, X3D_Vex2D* b) {
  return a->x == b->x && a->y == b->y;
}

static inline X3D_Vex2D x3d_vex2d_add(X3D_Vex2D* a, X3D_Vex2D* b) {
    return x3d_vex2d_make(a->x + b->x, a->y + b->y);
}

static inline X3D_Vex2D x3d_vex2d_sub(X3D_Vex2D* a, X3D_Vex2D* b) {
    return x3d_vex2d_make(a->x - b->x, a->y - b->y);
}

static inline X3D_Vex3D_int32 x3d_vex3d_int32_add_vex3d(X3D_Vex3D_int32* a, X3D_Vex3D* b) {
    return (X3D_Vex3D_int32) {
        a->x + b->x,
        a->y + b->y,
        a->z + b->z
    };
}

static inline X3D_Vex3D x3d_vex3d_int32_div_by_int16_as_vex3d(X3D_Vex3D_int32* v, int16 divisor) {
    return (X3D_Vex3D) {
        v->x / divisor,
        v->y / divisor,
        v->z / divisor
    };
}

static inline X3D_Vex3D_int32 x3d_vex3d_int32_origin() {
    return (X3D_Vex3D_int32) { 0, 0, 0 };
}

static inline _Bool x3d_vex3d_in_front_of_near_plane(X3D_Vex3D* v) {
  /// @todo Replace with near plane constant
  return v->z > 10;
}


static inline void x3d_vex3d_fp0x16_mul_by_int16(X3D_Vex3D* v, int16 i16, X3D_Vex3D* dest) {
  dest->x = mul_fp0x16_by_int16_as_int16(v->x, i16);
  dest->y = mul_fp0x16_by_int16_as_int16(v->y, i16);
  dest->z = mul_fp0x16_by_int16_as_int16(v->z, i16);
}

static inline X3D_Vex3D x3d_vex3d_make(int16 x, int16 y, int16 z) {
  return (X3D_Vex3D) { x, y, z };
}

static inline int16 x3d_vex3d_distance(X3D_Vex3D* a, X3D_Vex3D* b) {
  X3D_Vex3D diff = x3d_vex3d_sub(a, b);
  return x3d_vex3d_int16_mag(&diff);
}

static inline X3D_Vex3D_fp16x8 x3d_vex3d_fp16x8_add_vex3d_fp8x8(X3D_Vex3D_fp16x8* v16x8, X3D_Vex3D_fp8x8* v8x8) {
  return (X3D_Vex3D_fp16x8) {
    v16x8->x + v8x8->x,
    v16x8->y + v8x8->y,
    v16x8->z + v8x8->z
  };
}

static inline X3D_Vex3D x3d_vex3d_fp16x8_to_vex3d(X3D_Vex3D_fp16x8* v) {
  return x3d_vex3d_make(v->x >> 8, v->y >> 8, v->z >> 8);
}

static inline X3D_Vex3D x3d_vex3d_abs(X3D_Vex3D* v) {
    return x3d_vex3d_make(abs(v->x), abs(v->y), abs(v->z));
}

static inline X3D_Vex3D_int32 x3d_vex3d_int32_make(int32 x, int32 y, int32 z) {
    return (X3D_Vex3D_int32) { x, y, z };
}

static inline X3D_Vex3D_fp16x16 x3d_vex3d_fp16x16_make(fp16x16 x, fp16x16 y, fp16x16 z) {
    return x3d_vex3d_int32_make(x, y, z);
}

static inline X3D_Vex3D_fp16x16 x3d_vex3d_to_vex3d_fp16x16(X3D_Vex3D* v) {
    return x3d_vex3d_fp16x16_make(
        (fp16x16)v->x << 16,
        (fp16x16)v->y << 16,
        (fp16x16)v->z << 16
    );
}

