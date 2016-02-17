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

#include <string.h>
#include <stdio.h>

#include "X3D_common.h"
#include "X3D_matrix.h"
#include "X3D_vector.h"
#include "X3D_trig.h"

///////////////////////////////////////////////////////////////////////////////
/// Multiplies two fp0x16 matricies together (aka matrix concatenation).
///
/// @param dest - pointer to the destination matrix
/// @param a    - pointer to the first matrix to multiply
/// @param b    - pointer to the second matrix to multiply
///
/// @return nothing
/// @note dest must not be an alias for a or b
///////////////////////////////////////////////////////////////////////////////
void x3d_mat3x3_mul(X3D_Mat3x3* dest, X3D_Mat3x3* a, X3D_Mat3x3* b) {
  int i, j, k;

  for(i = 0; i < 9; i++)
    dest->data[i] = 0;

  for(i = 0; i < 3; i++) {
    for(j = 0; j < 3; j++) {
      for(k = 0; k < 3; k++) {
        dest->data[MAT3x3(i, j)] += x3d_fp0x16_mul(a->data[MAT3x3(i, k)], b->data[MAT3x3(k, j)]);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs an fp0x16 3D rotation matrix from a set of Euler anglers.
///
/// @param dest    - pointer to the destination matrix
/// @param angle   - pointer to Vex3D of angles describing rotation around
///    x, y, and z axis.
///
/// @return nothing
/// @todo Add support for rotation around Z axis.
/// @todo Construct matrix using product-to-sum identities instead of matrix
///    multiplication.
///////////////////////////////////////////////////////////////////////////////
void x3d_mat3x3_construct(X3D_Mat3x3_fp0x16 *dest, X3D_Vex3D_angle256 *angle) {
#if 1

  fp0x16 sin_y = x3d_sin(angle->y);
  fp0x16 cos_y = x3d_cos(angle->y);

  fp0x16 sin_x = x3d_sin(angle->x);
  fp0x16 cos_x = x3d_cos(angle->x);

  X3D_Mat3x3 mat_y = {
    {
      cos_y, 0, sin_y,
      0, INT16_MAX, 0,
      -sin_y, 0, cos_y
    }
  };

  X3D_Mat3x3 mat_x = {
    {
      INT16_MAX, 0, 0,
      0, cos_x, -sin_x,
      0, sin_x, cos_x,
    }
  };

  X3D_Mat3x3 mul_res;

  x3d_mat3x3_mul(&mul_res, &mat_y, &mat_x);

  memcpy(dest, &mul_res, sizeof(X3D_Mat3x3));
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// Prints out a 3x3 matrix.
///
/// @param mat  - the matrix to print out
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_mat3x3_print(X3D_Mat3x3* mat) {
  uint16 r;

  for(r = 0; r < 3; ++r) {
    printf("%d %d %d\n", mat->data[MAT3x3(r, 0)], mat->data[MAT3x3(r, 1)], mat->data[MAT3x3(r, 2)]);
  }
}

/// @todo Document.
void x3d_mat3x3_transpose(X3D_Mat3x3* mat) {
  X3D_Mat3x3 temp;

  uint16 i, j;

  for(i = 0; i < 3; ++i) {
    for(j = 0; j < 3; ++j) {
      temp.data[i * 3 + j] = mat->data[j * 3 + i];
    }
  }

  *mat = temp;
}

void x3d_mat3x3_set_column(X3D_Mat3x3* mat, int16 col, X3D_Vex3D_fp0x16* v) {
  mat->data[col] = v->x;
  mat->data[col + 3] = v->y;
  mat->data[col + 6] = v->z;
}

void x3d_mat3x3_get_column(X3D_Mat3x3* mat, int16 col, X3D_Vex3D_fp0x16* dest) {
  dest->x = mat->data[col];
  dest->y = mat->data[col + 3];
  dest->z = mat->data[col + 6];
}

void x3d_mat3x3_set_row(X3D_Mat3x3* mat, int16 row, X3D_Vex3D_fp0x16* v) {
  mat->data[row * 3] = v->x;
  mat->data[row * 3 + 1] = v->y;
  mat->data[row * 3 + 2] = v->z;
}

void x3d_mat3x3_get_row(X3D_Mat3x3* mat, int16 row, X3D_Vex3D_fp0x16* dest) {
  dest->x = mat->data[row * 3];
  dest->y = mat->data[row * 3 + 1];
  dest->z = mat->data[row * 3 + 2];
}

#if 0

fp8x8 x3d_fp8x8_mul(fp8x8 a, fp8x8 b) {
  
}

#endif


void x3d_mat3x3_from_axis_angle(X3D_Mat3x3* dest, X3D_Vex3D* axis, angle256 angle) {
#if 0
  fp8x8 c = x3d_cos(angle) >> 7;
  fp8x8 s = x3d_sin(angle) >> 7;
  fp8x8 C = 256 - C;
  
  fp8x8 xx = x3d_fp8x8_mul(axis->x, axis->x);
  fp8x8 xy = x3d_fp8x8_mul(axis->x, axis->y);
  fp8x8 xz = x3d_fp8x8_mul(axis->x, axis->z);
  
  fp8x8 yy = x3d_fp8x8_mul(axis->y, axis->y);
  fp8x8 yz = x3d_fp8x8_mul(axis->y, axis->z);
  
  fp8x8 zz = x3d_fp8x8_mul(axis->z, axis->z);
  
  fp8x8 zs = x3d_fp8x8_mul(axis->z, 
  
  
  dest->data[0] = x3d_fp8x8_mul(xx, C) + c;
  dest->data[1] = x3d_fp8x8_mul(xy, C) + c;
#endif
}

