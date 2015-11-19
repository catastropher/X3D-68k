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

#include "X3D_fix.h"

//=============================================================================
// Defines
//=============================================================================

//< Macro for giving 1D array index from a 3x3 matrix row and column
#define MAT3x3(_row, _col) ((_row) * 3 + (_col))

//=============================================================================
// Structures
//=============================================================================

/// A 3x3 matrix with fp0x16 elements
typedef struct X3D_Mat3x3_fp0x16 {
  fp0x16 data[9];
} X3D_Mat3x3_fp0x16;

//=============================================================================
// Function declarations
//=============================================================================

void x3d_mat3x3_fp0x16_mul(X3D_Mat3x3_fp0x16* dest, X3D_Mat3x3_fp0x16* a, X3D_Mat3x3_fp0x16* b);
void x3d_mat3x3_fp0x16_print(X3D_Mat3x3_fp0x16* mat);
void x3d_mat3x3_fp0x16_construct(X3D_Mat3x3_fp0x16 *dest, Vex3D_angle256 *angle);
