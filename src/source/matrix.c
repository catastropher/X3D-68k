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

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_matrix.h"
#include "X3D_trig.h"

/// Temporary fixed point multiplication until Jason completes his work.
static fp0x16 temp_x3d_fp0x16_mul(fp0x16 a, fp0x16 b) {
  return ((int32)a * b) >> 15;
}

/**
 * Multiplies two fp0x16 matricies together (aka matrix concatenation).
 *
 * @param dest - pointer to the destination matrix
 * @param a    - pointer to the first matrix to multiply
 * @param b    - pointer to the second matrix to multiply
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

/**
 * Constructs an fp0x16 3D rotation matrix from a set of Euler anglers.
 *
 * @param dest    - pointer to the destination matrix
 * @param angle   - pointer to Vex3D of angles describing rotation around
 *		x, y, and z axis.
 *
 * @return nothing
 * @todo Add support for rotation around Z axis.
 * @todo Construct matrix using product-to-sum identities instead of matrix
 *		multiplication.
 */
void x3d_mat3x3_fp0x16_construct(X3D_Mat3x3_fp0x16 *dest, X3D_Vex3D_angle256 *angle) {
#if 1

  fp0x16 sin_y = x3d_sinfp(angle->y);
  fp0x16 cos_y = x3d_cosfp(angle->y);

  fp0x16 sin_x = x3d_sinfp(angle->x);
  fp0x16 cos_x = x3d_cosfp(angle->x);

  X3D_Mat3x3_fp0x16 mat_y = {
    {
      cos_y, 0, sin_y,
      0, INT16_MAX, 0,
      -sin_y, 0, cos_y
    }
  };

  X3D_Mat3x3_fp0x16 mat_x = {
    {
      INT16_MAX, 0, 0,
      0, cos_x, -sin_x,
      0, sin_x, cos_x,
    }
  };

  X3D_Mat3x3_fp0x16 mul_res;

  x3d_mat3x3_fp0x16_mul(&mul_res, &mat_y, &mat_x);

  memcpy(dest, &mul_res, sizeof(X3D_Mat3x3_fp0x16));
#endif
}

/// @todo document
void x3d_mat3x3_fp0x16_print(X3D_Mat3x3_fp0x16* mat) {
  uint16 r;

  for(r = 0; r < 3; ++r) {
    printf("%d %d %d\n", mat->data[MAT3x3(r, 0)], mat->data[MAT3x3(r, 1)], mat->data[MAT3x3(r, 2)]);
  }
}

