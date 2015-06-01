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

#ifndef VECTOR_H
#define VECTOR_H

#include "X3D_fix.h"

//=============================================================================
// Defines
//=============================================================================

/// The number of fractional bits used to represent a normal in fixed point.
/// All normals are in 0.16 format.
#define X3D_NORMAL_SHIFT 15

//=============================================================================
// Structures
//=============================================================================

typedef struct X3D_Vex3D_int16 {
  int16 x;
  int16 y;
  int16 z;
} X3D_Vex3D_int16;

typedef X3D_Vex3D_int16 X3D_Vex3D_fp8x8;
typedef X3D_Vex3D_int16 X3D_Vex3D_fp0x16;

typedef struct Vex2D {
  int16 x;
  int16 y;
} Vex2D;

//=============================================================================
// Function declarations
//=============================================================================
int32 x3d_vex3d_int16_dot(X3D_Vex3D_int16* a, X3D_Vex3D_int16* b);
int16 x3d_vex3d_fp0x16_dot(X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b);
inline void x3d_vex3d_fp0x16_normalize(X3D_Vex3D_fp0x16* v);
void x3d_print_vex3d_int16(X3D_Vex3D_int16* v);


//=============================================================================
// Inline functions
//=============================================================================

/**
 *  Adds two 3D vectors.
 *
 *  @param a     - pointer to the first 3D vector
 *  @param b     - pointer to the second 3D vector
 *  @param dest  - pointer to the destination 3D vector
 *
 *  @return nothing
 */
static inline X3D_Vex3D_int16 x3d_vex3d_int16_add(X3D_Vex3D_int16* a, X3D_Vex3D_int16* b) {
  return (X3D_Vex3D_int16){a->x + b->x, a->y + b->y, a->z + b->z};
}

/**
*  Subtracts two 3D vectors.
*
*  @param a     - pointer to the first 3D vector
*  @param b     - pointer to the second 3D vector
*  @param dest  - pointer to the destination 3D vector
*
*  @return nothing
*/
static inline X3D_Vex3D_int16 x3d_vex3d_int16_sub(X3D_Vex3D_int16* a, X3D_Vex3D_int16* b) {
  return (X3D_Vex3D_int16){a->x - b->x, a->y - b->y, a->z - b->z};
}

#endif

