// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "X3D_const.h"
#include "X3D_fix.h"

//=============================================================================
// Defines
//=============================================================================

#ifndef INT16_MAX
#define INT16_MAX 32767
#endif

//=============================================================================
// Forward declarations
//=============================================================================

struct X3D_RenderContext;
struct X3D_Mat3x3_fp0x16;

//=============================================================================
// Structures
//=============================================================================

/// A 3D vector with int16 components.
typedef struct X3D_Vex3D_int16 {
  int16 x;
  int16 y;
  int16 z;
} X3D_Vex3D_int16;

/// A 3D vector with int32 components.
typedef struct X3D_Vex3D_int32 {
  int32 x;
  int32 y;
  int32 z;
} X3D_Vex3D_int32;

typedef X3D_Vex3D_int16 Vex3D;

/// A 3D vector with fp8x8 components.
typedef X3D_Vex3D_int16 X3D_Vex3D_fp8x8;

/// A shorthand for X3D_Vex3D_int16
typedef X3D_Vex3D_int16 X3D_Vex3D;

/// A 3D vector with fp0x16 components.
typedef X3D_Vex3D_int16 X3D_Vex3D_fp0x16;

/// A 3D vector with fp16x16 components.
typedef X3D_Vex3D_int32 X3D_Vex3D_fp16x16;

/// A 3D vector with uint8 components.
typedef struct X3D_Vex3D_uint8 {
  uint8 x;
  uint8 y;
  uint8 z;
} X3D_Vex3D_uint8;

/// A 3D vector with angle256 components.
typedef struct X3D_Vex3D_uint8 X3D_Vex3D_angle256;

/// A 2D vector with int16 components.
typedef struct X3D_Vex2D_int16 {
  int16 x;
  int16 y;
} X3D_Vex2D_int16;

/// A shorthand for X3D_Vex2D_int16
typedef X3D_Vex2D_int16 X3D_Vex2D;

//=============================================================================
// Function declarations
//=============================================================================

int32 x3d_vex3d_int16_dot(X3D_Vex3D_int16* a, X3D_Vex3D_int16* b);
int16 x3d_vex3d_fp0x16_dot(X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b);
inline void x3d_vex3d_fp0x16_normalize(X3D_Vex3D_fp0x16* v);
void x3d_print_vex3d_int16(X3D_Vex3D_int16* v);

void x3d_vex3d_int16_print(X3D_Vex3D_int16* v);
void x3d_vex3d_int16_project(X3D_Vex2D_int16* dest, const X3D_Vex3D_int16* src, struct X3D_RenderContext* context);
void x3d_vex3d_int16_rotate(X3D_Vex3D_int16* dest, X3D_Vex3D_int16* src, struct X3D_Mat3x3_fp0x16* mat);
void x3d_vex3d_fp0x16_cross(X3D_Vex3D_fp0x16* dest, X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b);
uint16 x3d_vex3d_int16_mag(X3D_Vex3D_int16* v);


//=============================================================================
// Inline functions
//=============================================================================

/**
 *  Adds two 3D vectors.
 *
 *  @param a     - pointer to the first 3D vector
 *  @param b     - pointer to the second 3D vector
 *
 *  @return a + b as an X3D_Vex3D_int16
 */
static inline X3D_Vex3D_int16 vex3d_int16_add(X3D_Vex3D_int16* a, X3D_Vex3D_int16* b) {
  X3D_STACK_TRACE;

  return (X3D_Vex3D_int16) { a->x + b->x, a->y + b->y, a->z + b->z };
}

/**
*  Subtracts two 3D vectors.
*
*  @param a     - pointer to the first 3D vector
*  @param b     - pointer to the second 3D vector
*
*  @return a + b as an X3D_Vex3D_int16
*/
static inline X3D_Vex3D_int16 vex3d_int16_sub(X3D_Vex3D_int16* a, X3D_Vex3D_int16* b) {
  X3D_STACK_TRACE;

  return (X3D_Vex3D_int16) { a->x - b->x, a->y - b->y, a->z - b->z };
}

static inline X3D_Vex3D_int16 vneg16(Vex3D* v) {
  return (Vex3D) { -v->x, -v->y, -v->z };
}

static inline X3D_Vex3D_int16 vscale16(Vex3D* v, fp8x8 scale) {
  return (Vex3D) {
    ((int32)scale * v->x) >> 8,
    ((int32)scale * v->y) >> 8,
    ((int32)scale * v->z) >> 8
  };
}

