/* This file is part of X3D.
 *
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "X3D_config.h"
//#include "X3D_const.h"
#include "X3D_fix.h"

//=============================================================================
// Defines
//=============================================================================

//=============================================================================
// Forward declarations
//=============================================================================

struct X3D_RenderContext;
struct X3D_Mat3x3_fp0x16;

//=============================================================================
// Structures
//=============================================================================

/// A 3D vector with int16 components.
typedef struct Vex3D_int16 {
  int16 x;
  int16 y;
  int16 z;
} Vex3D_int16;

/// A 3D vector with int32 components.
typedef struct Vex3D_int32 {
  int32 x;
  int32 y;
  int32 z;
} Vex3D_int32;

/// A 3D vector with fp8x8 components.
typedef Vex3D_int16 Vex3D_fp8x8;

/// A shorthand for X3D_Vex3D_int16
typedef Vex3D_int16 Vex3D;

/// A 3D vector with fp0x16 components.
typedef Vex3D_int16 Vex3D_fp0x16;

/// A 3D vector with fp16x16 components.
typedef Vex3D_int32 Vex3D_fp16x16;

/// A 3D vector with uint8 components.
typedef struct Vex3D_uint8 {
  uint8 x;
  uint8 y;
  uint8 z;
} Vex3D_uint8;

/// A 3D vector with angle256 components.
typedef struct Vex3D_uint8 Vex3D_angle256;

/// A 2D vector with int16 components.
typedef struct Vex2D_int16 {
  int16 x;
  int16 y;
} Vex2D_int16;

/// A shorthand for Vex2D_int16
typedef Vex2D_int16 Vex2D;

//=============================================================================
// Function declarations
//=============================================================================


//=============================================================================
// Inline functions
//=============================================================================




#define V3ADD(_a, _b) ((typeof(_a)){_a.x + _b.x, _a.y + _b.y, _a.z + _b.z})
#define V3SUB(_a, _b) ((typeof(_a)){_a.x - _b.x, _a.y - _b.y, _a.z - _b.z})




