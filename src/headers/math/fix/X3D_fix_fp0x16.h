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

#include "X3D_common.h"
#include "X3D_fix_types.h"

/*
typedef int8    fp0x8;
typedef uint8   ufp0x8;

typedef int16   fp8x8;
typedef uint16  ufp8x8;

typedef int32   fp16x16;
typedef uint32  ufp16x16;

typedef int16   fp0x16;
typedef uint16  ufp0x16;

typedef int32   fp0x32;
typedef uint32  ufp0x32;
*/

#define X3D_FP0x16_ONE_HALF     16384
#define X3D_FP0x16_ONE_FOURTH   8192
#define X3D_FP0x16 ONE_EIGHTH   4192

#define X3D_FP0x16_MAX 32767
#define X3D_FP0x16_MIN -32768

static inline fp0x16 x3d_fp0x16_from_float(float f) {
  if(f >= 0)
    return X3D_MIN(f * 32768, X3D_FP0x16_MAX);
  else
    return X3D_MAX(f * 32768, X3D_FP0x16_MIN);
}

static inline fp0x16 x3d_fp0x16_from_fp0x8(fp0x8 fp) {
  return fp << 7;
}

static inline fp0x16 x3d_fp0x16_from_fp0x32(fp0x32 fp) {
  return fp >> 16;
}

static inline fp0x16 x3d_fp0x16_from_fp0x64(fp0x64 fp) {
  return fp >> 48;
}


static inline fp0x16 x3d_fp0x16_mul_by_fp0x16_as_fp0x16(fp0x16 a, fp0x16 b) {
  return ((int32)a * b) >> 15;
}

static inline int16 x3d_fp0x16_mul_by_int16_as_int16(fp0x16 a, int16 b) {
  return ((int32)a * b) >> 15;
}

static inline fp0x16 x3d_fp0x16_mul_by_int16_as_fp0x16(fp0x16 a, int16 b) {
  return a * b;
}


static inline fp16x16 x3d_fp0x16_div_by_fp0x16_as_fp16x16(fp0x16 a, fp0x16 b) {
  return 0;
}

static inline fp0x16 x3d_fp0x16_div_by_int16_as_fp0x16(fp0x16 a, fp0x16 b) {
  return 0;
}

