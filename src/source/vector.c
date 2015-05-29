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
#include "X3D_vector.h"
#include "X3D_error.h"

/**
* Calculates the dot product of two 16-bit integer 3D vectors.
*
* @param a - pointer to the first 3D vector
* @param b - pointer to the the second 3D vector
*
* @return dot product of a and b as an int32
*/
int32 x3d_vex3d_int16_dot(X3D_Vex3D_int16* a, X3D_Vex3D_int16* b) {
#if 0
#ifdef X3D_CHECK_OVERFLOW

  int32 x = (int32)a->x * b->y;
  int32 y = (int32)a->y * b->y;
  int32 z = (int32)a->z * b->z;

  int32 xplusy, result;

  if(!__builtin_saddl_overflow(x, y, &xplusy)) {
    if(!__builtin_saddl_overflow(xplusy, z, &result)) {
      return result;
    }
  }

  x3d_error("int16 dot product overflow\na: {%d, %d, %d}\nb: {%d, %d, %d}",
    a->x, a->y, a->z, b->x, b->y, b->z);

#else

  return a->x * b->x + (int32)a->y * b->y + (int32)a->z * b->z;

#endif
#endif
  return 0;
}

// @todo comment
void x3d_print_vex3d_int16(X3D_Vex3D_int16* v) {
  printf("{%d, %d, %d}\n", v->x, v->y, v->z);
}

/**
* Calculates the dot product of two 0x16 fixed-point 3D vectors.
*
* @param a - pointer to the first 3D vector
* @param b - pointer to the second 3D vector
*
* @return dot product of a and b as a long
*
* @note This returns the unshifted dot product.
*/
int16 x3d_vex3d_fp0x16_dot(X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b) {
  return x3d_vex3d_int16_dot(a, b) >> X3D_NORMAL_BITS;
}

#if 0

/**
* Normalizes a 3D vector (makes the entire length 1). The result is in 0.15
*		format.
* @param v - pointer to the 3D vector to normalize
*
* @return nothing
*
* @note If src->x, src->y, and src->z are all zero, this cause a division by
*		zero (since we divide by the length).
*/
inline void x3d_normalize_vex3d(Vex3D* v) {
  int32 val =
    (((int32)v->x * v->x) >> 2) +
    (((int32)v->y * v->y) >> 2) +
    (((int32)v->z * v->z) >> 2);


  x3d_errorif(val < 0, "normalize overflow");


  uint16 len = (x3d_fastsqrt(val) << 1) + 1;

  x3d_errorif(X3D_SIGNOF(v->y) != X3D_SIGNOF(((long)v->y << NORMAL_BITS)), "Wrong sign");


  v->x = ((int32)v->x << NORMAL_BITS) / len;
  v->y = ((int32)v->y << NORMAL_BITS) / len;
  v->z = ((int32)v->z << NORMAL_BITS) / len;
}

#endif
