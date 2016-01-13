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

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_assert.h"
#include "X3D_enginestate.h"
#include "X3D_screen.h"
#include "X3D_matrix.h"
#include "X3D_fastsqrt.h"

///////////////////////////////////////////////////////////////////////////////
/// Calculates the dot product of two 16-bit integer 3D vectors.
///
/// @param a - pointer to the first 3D vector
/// @param b - pointer to the the second 3D vector
///
/// @return dot product of a and b as an int32
/// @note This function has the *potential* to overflow, but this has yet to be
///   seen in practice (it requires huge vectors). If overflow does happen, the
///   function calling this should scale down the vectors before calling this.
///////////////////////////////////////////////////////////////////////////////
int32 x3d_vex3d_int16_dot(X3D_Vex3D* a, X3D_Vex3D* b) {
  int32 p_x = (int32)a->x * b->x;
  int32 p_y = (int32)a->y * b->y;
  int32 p_z = (int32)a->z * b->z;
  
  // Check for overflow
  x3d_assert(!x3d_addi32_check_overflow(3, p_x, p_y, p_z));
  
  return p_x + p_y + p_z; 
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the dot product of two 0x16 fixed-point 3D vectors.
///
/// @param a - pointer to the first 3D vector
/// @param b - pointer to the second 3D vector
///
/// @return dot product of a and b as an fp0x16
///////////////////////////////////////////////////////////////////////////////
fp0x16 x3d_vex3d_fp0x16_dot(X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b) {
  return x3d_vex3d_int16_dot(a, b) >> X3D_NORMAL_BITS;
}

/// @todo clean up formatting

#if 0

void x3d_normalize_vex2d_fp0x16(Vex2D_fp0x16* v) {
  int32 distance_squared = 
    (((int32)v->x * v->x)) +
    (((int32)v->y * v->y));
    
  // If distance_squared is negative, the only possible explaination is that
  // it overflowed (should never happen because we divide each term by 4)
  x3d_errorif(distance_squared <= 0, "normalize overflow");
  
  
  // Calculate the actual length of the vector
  // Notice that we multiply the square root by 2 and add 1: we multiply by 2
  // because distance_squared is actually (x^2 + y^2 + z^2) / 4, so
  // sqrt((x^2 + y^2 + z^2) / 4) = .5 * sqrt(x^2 + y^2 + z^2). Thus, we multiply
  // by 2 to get the real result. We add 1 to prevent division by 0 and to make
  // sure we never get 0x8000 after the division (because that's to big to fit in
  // an int16)
  uint16 len = (x3d_fastsqrt(distance_squared));
  
  
  
  /// @todo Add check to make sure the sign of the components is the same after
  /// dividing?
  
  if(len == 0) {
    x3d_error("NORMALIZE 2D ERROR\n");
  }
  
  // Divide each component by the length
  /// @todo Replace with fixed point division
  
  //x3d_errorif(abs(v->x) == len, "v->x == len");
  //x3d_errorif(abs(v->y) == len, "v->y == len");
  
  if(abs(v->y) == len) {
    if(v->y < 0) {
      v->y = -0x7FFF;
    }
    else {
      v->y = 0x7FFF;
    }
  }
  else {
    v->y = ((int32)v->y << X3D_NORMAL_SHIFT) / len;
  }
  
  
  
  if(abs(v->x) == len) {
    if(v->x < 0) {
      v->x = -0x7FFF;
    }
    else {
      v->x = 0x7FFF;
    }
  }
  else {
    v->x = ((int32)v->x << X3D_NORMAL_SHIFT) / len;
  }
}

#endif

/**
 * Normalizes an fp0x16 3D vector (makes the entire length 1).
 *
 * @param v - pointer to the 3D vector to normalize
 *
 * @return nothing
 */
inline void x3d_vex3d_fp0x16_normalize(X3D_Vex3D_fp0x16* v) {
  // Calculate x^2 + y^2 + z^2 for the distance formula.
  //
  // We divide each term by 4 to prevent overflow (if x, y, and z are all 0x7FFF,
  // 0x3FFF0001 + 0x3FFF0001 + 0x3FFF0001 = 0xBFFD0003, which is too big to fit
  // in an int32. However, dividing each term by 4 would give
  // 0x0FFFC000 + 0x0FFFC000 + 0x0FFFC000 = 0x2FFF4000, which fits. Since scaling
  // a vector changes its magnitude but not direction (and in the end we want the
  // length to be 1 anyway), we can do this.
  //
  // Note: we can't divide by 4 after adding because by then it may have already
  // overflowed!
  int32 distance_squared =
    (((int32)v->x * v->x) >> 2) +
    (((int32)v->y * v->y) >> 2) +
    (((int32)v->z * v->z) >> 2);


  // If distance_squared is negative, the only possible explaination is that
  // it overflowed (should never happen because we divide each term by 4)
  x3d_assert(distance_squared >= 0);    

  // Calculate the actual length of the vector
  // Notice that we multiply the square root by 2 and add 1: we multiply by 2
  // because distance_squared is actually (x^2 + y^2 + z^2) / 4, so
  // sqrt((x^2 + y^2 + z^2) / 4) = .5 * sqrt(x^2 + y^2 + z^2). Thus, we multiply
  // by 2 to get the real result. We add 1 to prevent division by 0 and to make
  // sure we never get 0x8000 after the division (because that's to big to fit in
  // an int16)
  uint16 len = (x3d_fastsqrt(distance_squared) << 1) + 1;


  /// @todo Add check to make sure the sign of the components is the same after
  /// dividing?

  // Divide each component by the length
  /// @todo Replace with fixed point division
  v->x = ((int32)v->x * (1 << X3D_NORMAL_BITS)) / len;
  v->y = ((int32)v->y * (1 << X3D_NORMAL_BITS)) / len;
  v->z = ((int32)v->z * (1 << X3D_NORMAL_BITS)) / len;
}

/**
 * Prints out an int16 Vex3D on the screen.
 *
 * @param v - pointer to the 3D vector to print.
 *
 * @return nothing
 */
void x3d_vex3d_int16_print(X3D_Vex3D_int16* v) {
 // printf("{%d, %d, %d}\n", v->x, v->y, v->z);
}


/**
* Projects a 3D point onto a screen.
*
* @param dest - destination vector
* @param src - source vector
* @param context - rendering context
*
* @return nothing
* @note If src->z is 0, dest->x and dest->y are set to 0 to prevent division
*     by 0.
*/
void x3d_vex3d_int16_project(X3D_Vex2D_int16* dest, const X3D_Vex3D_int16* src) {
  const X3D_ScreenManager* screen = x3d_screenmanager_get();
  
  // To prevent division by zero
  if(src->z == 0) {
    dest->x = 0;
    dest->y = 0;
  }
  else {
    // @todo Replace division by src->z with fixed point multiply
    dest->x = ((int32)src->x * screen->scale) / src->z + screen->center.x;
    dest->y = ((int32)src->y * screen->scale) / src->z + screen->center.y;
  }
}


/**
* Rotates a 3D vector around the origin.
*
* @param dest - destination vector
* @param src - source vector
* @param mat - fp0x16 rotation matrix
*
* @return nothing
* @todo Replace with assembly version
*/
void x3d_vex3d_int16_rotate(X3D_Vex3D_int16* dest, X3D_Vex3D_int16* src, X3D_Mat3x3* mat) {
  fp0x16* m = mat->data;
  
  X3D_Vex3D_int16 x = (X3D_Vex3D_int16){ m[MAT3x3(0, 0)], m[MAT3x3(1, 0)], m[MAT3x3(2, 0)] };
  X3D_Vex3D_int16 y = (X3D_Vex3D_int16){ m[MAT3x3(0, 1)], m[MAT3x3(1, 1)], m[MAT3x3(2, 1)] };
  X3D_Vex3D_int16 z = (X3D_Vex3D_int16){ m[MAT3x3(0, 2)], m[MAT3x3(1, 2)], m[MAT3x3(2, 2)] };

  dest->x = x3d_vex3d_fp0x16_dot(&x, src);
  dest->y = x3d_vex3d_fp0x16_dot(&y, src);
  dest->z = x3d_vex3d_fp0x16_dot(&z, src);
}

// Calculates the cross product of two vectors. This creates a vector that
// is perpendicular to both vectors
// Note: this routine will normalize the result
void x3d_vex3d_fp0x16_cross(X3D_Vex3D_fp0x16* dest, X3D_Vex3D_fp0x16* a, X3D_Vex3D_fp0x16* b) {
  int32 xxx = ((((int32)a->y * b->z) >> 1) - (((int32)a->z * b->y) >> 1));
  int32 yyy = ((((int32)a->z * b->x) >> 1) - (((int32)a->x * b->z) >> 1));
  int32 zzz = ((((int32)a->x * b->y) >> 1) - (((int32)a->y * b->x) >> 1));

  while(abs(xxx) >= 0x7FFF || abs(yyy) >= 0x7FFF || abs(zzz) >= 0x7FFF) {
    xxx >>= 1;
    yyy >>= 1;
    zzz >>= 1;
  }

  dest->x = xxx;
  dest->y = yyy;
  dest->z = zzz;

  x3d_vex3d_fp0x16_normalize(dest);
}

/// @todo document
uint16 x3d_vex3d_int16_mag(X3D_Vex3D_int16* v) {
  int32 distance_squared =
    (((int32)v->x * v->x) >> 2) +
    (((int32)v->y * v->y) >> 2) +
    (((int32)v->z * v->z) >> 2);

  return (x3d_fastsqrt(distance_squared) << 1) + 1;
}
