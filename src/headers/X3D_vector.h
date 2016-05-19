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

typedef X3D_Vex3D_int16 X3D_Vex3D;
typedef X3D_Vex3D_int16 X3D_Vex3D_fp0x16;
typedef X3D_Vex3D_int16 X3D_Vex3D_fp8x8;

typedef X3D_Vex2D_int16 X3D_Vex2D;

typedef X3D_Vex3D_uint8 X3D_Vex3D_angle256;

typedef X3D_Vex3D_int32 X3D_Vex3D_fp16x8;

struct X3D_Mat3x3_fp0x16;

void x3d_vex3d_int16_project(X3D_Vex2D_int16* dest, const X3D_Vex3D_int16* src);
void x3d_vex3d_int16_rotate(X3D_Vex3D_int16* dest, X3D_Vex3D_int16* src, struct X3D_Mat3x3_fp0x16* mat);
void x3d_vex3d_fp0x16_cross(X3D_Vex3D_fp0x16* dest, X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b);
int32 x3d_vex3d_int16_dot(X3D_Vex3D* a, X3D_Vex3D* b);
fp0x16 x3d_vex3d_fp0x16_dot(X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b);
void x3d_vex3d_fp0x16_normalize(X3D_Vex3D_fp0x16* v);
uint16 x3d_vex3d_int16_mag(X3D_Vex3D_int16* v);

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

