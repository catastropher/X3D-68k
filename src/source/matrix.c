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

#include "X3D_fix.h"
#include "X3D_matrix.h"

/// Temporary fixed point multiplication until Jason completes his work
static fp0x16 temp_x3d_fp0x16_mul(fp0x16 a, fp0x16 b) {
  return ((int32)a * b) >> 16;
}

// Multiplies two 3x3 matricies i.e. concatenates them

/**
* Multiplies two fp0x16 matricies together (aka matrix concatenation).
*
* @param res - pointer to the destination matrix
* @param a   - pointer to the first matrix to multiply
* @param b   - pointer to the second matrix to multiply
*
* @return nothing
* @note dest must not be an alias for a or b
*/
void x3d_mat3x3_fp0x16_mul(X3D_Mat3x3_fp0x16* dest, X3D_Mat3x3_fp0x16* a, X3D_Mat3x3_fp0x16* b) {
  int i, j, k;

  for(i = 0; i < 9; i++) {
    dest->data[i] = 0;
  }

  for(i = 0; i < 3; i++) {
    for(j = 0; j < 3; j++) {
      for(k = 0; k < 3; k++) {
        dest->data[MAT3x3(i, j)] += temp_x3d_fp0x16_mul(a->data[MAT3x3(i, k)], b->data[MAT3x3(k, j)]);
      }
    }
  }
}

