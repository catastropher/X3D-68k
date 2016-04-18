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

#pragma once

#include "X3D_int.h"

/// An 8.8 fixed point number
typedef int16 fp8x8;

/// A 16.16 fixed point number
typedef int32 fp16x16;

/// A 0.16 fixed point number
typedef int16 fp0x16;

/// A 0.32 fixed point number
typedef int32 fp0x32;

/// An unsigned 8.8 fixed point number
typedef uint16 ufp8x8;

/// An angle that goes from 0-255 instead of 0-359
typedef uint8 angle256;


typedef struct x3d_fix_slope {
  int32 val;
  int16 shift;
} x3d_fix_slope;

static inline _Bool x3d_msb(int32 i) {
  return i & (1L << 30);
}

static inline void x3d_fix_slope_init(x3d_fix_slope* slope, int32 start, int32 end, int16 dx) {
  slope->shift = 0;
  
  
  if(start != 0 || end != 0) {
  #if 1
    while(!x3d_msb(start) && !x3d_msb(end)) {
      ++slope->shift;
      start <<= 1;
      end <<= 1;
    }
  #endif
  }
  
  slope->val = (end - start) / dx;
}

static inline void x3d_fix_slope_same_shift(x3d_fix_slope* slope, x3d_fix_slope* from, int32 val) {
  slope->shift = from->shift;
  slope->val = val << from->shift;
}

static inline void x3d_fix_slope_add(x3d_fix_slope* a, x3d_fix_slope* b) {
  a->val += b->val;
}

static inline int32 x3d_fix_slope_val(x3d_fix_slope* s) {
  return s->val >> s->shift;
}





///////////////////////////////////////////////////////////////////////////////
/// Multiplies two fp0x16 numbers.
///
/// @param a - first fixed point number
/// @param b - second fixed point number
///
/// @return Product of a and b
///////////////////////////////////////////////////////////////////////////////
static inline fp0x16 x3d_fp0x16_mul(fp0x16 a, fp0x16 b) {
  return ((fp0x32)a * b) >> 15;
}

/// @todo clean up the formatting
/**
* Divides an fp0x16 by an fp0x16.
*
* @param n - numerator
* @param d - denominator
*
* @return a / b as an fp8x8
*/
static inline fp8x8 div_fp0x16_by_fp0x16(fp0x16 n, fp0x16 d) {
  return ((int32)n << 8) / d;
}

/**
* Divides an int16 by an fp0x16.
*
* @param n - numerator
* @param d - denominator
*
* @return a / b as an int16
*/
static inline int16 div_int16_by_fp0x16(int16 n, fp0x16 d) {
  return ((int32)n << 8) / d;
}

/**
* Multiplies an fp0x16 by an int16.
*
* @param a - the fp0x16
* @param b - the int16
*
* @return a * b as an int16
*/
static inline int16 mul_fp0x16_by_int16_as_int16(fp0x16 a, int16 b) {
  return ((int32)a * b) >> 15;
}

/// @todo document
static inline fp8x8 div_int16_by_int16_as_fp8x8(int16 n, int16 d) {
  return ((int32)n << 8) / d;
}


static inline int16 x3d_fp16x6_whole(fp16x16 f) {
  return f >> 16;
}


