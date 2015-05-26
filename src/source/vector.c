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
* Calculates the short dot product of two 3D vectors.
*
* @param a - pointer to the first 3D vector
* @param b - pointer to the the second 3D vector
*
* @return dot product of a and b as a short
*
* @note This returns the dot product shifted left by @ref NORMAL_BITS to fit in
*   a short.
*/
int16 x3d_dot_product(Vex3D* a, Vex3D* b) {
  return ((int32)a->x * b->x + (int32)a->y * b->y + (int32)a->z * b->z) >> NORMAL_BITS;
}

/**
* Calculates the dot product of two 3D vectors.
*
* @param a - pointer to the first 3D vector
* @param b - pointer to the second 3D vector
*
* @return dot product of a and b as a long
*
* @note This returns the unshifted dot product.
*/
int32 x3d_dot_product_long(Vex3D* a, Vex3D* b) {
  return (int32)a->x * b->x + (int32)a->y * b->y + (int32)a->z * b->z;
}

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


