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
/// All normals are in 0.15 format.
#define NORMAL_BITS 15

//=============================================================================
// Structures
//=============================================================================

typedef struct Vex3D {
  int16 x;
  int16 y;
  int16 z;
} Vex3D;

typedef struct Vex2D {
  int16 x;
  int16 y;
} Vex2D;

//=============================================================================
// Function declarations
//=============================================================================
int16 dot_product(Vex3D* a, Vex3D* b);
int32 dot_product_long(Vex3D* a, Vex3D* b);

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
static void x3d_add_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest) {
  dest->x = a->x + b->x;
  dest->y = a->y + b->y;
  dest->z = a->z + b->z;
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
static void x3d_sub_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest) {
  dest->x = a->x - b->x;
  dest->y = a->y - b->y;
  dest->z = a->z - b->z;
}

#endif

