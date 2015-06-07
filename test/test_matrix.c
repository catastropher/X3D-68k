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

#include <X3D/X3D.h>

typedef double Mat3x3[3][3];

#define FIXMUL15(_a, _b) ((_a) * (_b))

// Multiplies two 3x3 matricies i.e. concatenates them
// (from the old X3D source code)
void mul_mat3x3(Mat3x3* a, Mat3x3* b, Mat3x3* res) {
  int i, j, k;

  for(i = 0; i < 3; i++)
  for(j = 0; j < 3; j++)
    (*res)[i][j] = 0;

  for(i = 0; i < 3; i++) {
    for(j = 0; j < 3; j++) {
      for(k = 0; k < 3; k++) {
        (*res)[i][j] += FIXMUL15((*a)[i][k], (*b)[k][j]);
      }
    }
  }
}

void move_mat3x3_to_fix(X3D_Mat3x3_fp0x16* dest, Mat3x3* src) {
  int16 r, c;

  for(r = 0; r < 3; ++r) {
    for(c = 0; c < 3; ++c) {
      dest->data[MAT3x3(r, c)] = (*src)[r][c] * (1 << 15);
    }
  }
}

void test_matrix() {
  Mat3x3 a = {
    { .75, .5, .25 },
    { .35, .95, .234 },
    { .99, .35, .75 }
  };

  Mat3x3 b = {
    {.5, .5, .25},
    { .8125, .75, .99 },
    {.235, .56, .825}
  };

  Mat3x3 res;

  mul_mat3x3(&a, &b, &res);

  X3D_Mat3x3_fp0x16 a_fix, b_fix, res_fix;

  move_mat3x3_to_fix(&a_fix, &a);
  move_mat3x3_to_fix(&b_fix, &b);

  x3d_mat3x3_fp0x16_mul(&res_fix, &a_fix, &b_fix);

}

