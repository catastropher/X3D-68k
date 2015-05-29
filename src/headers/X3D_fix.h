#pragma once
#define X3D_CHECK_OVERFLOW

#include "x3d_error.h"

/// An 8-bit signed integer
typedef char int8;

/// An 8-bit unsigned integer
typedef unsigned char uint8;

/// A 16-bit signed integer
typedef short int16;

/// A 16-bit unsigned integer
typedef unsigned short uint16;

/// A 32-bit signed integer
typedef long int32;

/// A 32-bit unsigned integer
typedef unsigned long uint32;

/// A 64-bit signed integer
typedef long long int64;

/// A 64-bit unsigned integer
typedef unsigned long long uint64;

/// An 8.8 fixed point number
typedef int16 fp8x8;

/// A 16.16 fixed point number
typedef int32 fp16x16;

/// A 0.16 fixed point number
typedef int16 fp0x16;

/// Returns the sign of a value
/// @todo Look into implementing as (_val >> ((sizeof(_val) * 8 - 1)
#define X3D_SIGNOF(_val) (_val < 0 ? -1 : (_val > 0 ? 1 : 0))

/**
 * Adds two 8x8 fixed point numbers.
 *
 * @param a   - first fixed point number
 * @param b   - second fixed point number
 *
 * @return sum of a and b as an fp8x8
 *
 * @note If @ref X3D_CHECK_OVERFLOW is defined, this will throw an error, if a + b overflows.
 */
static inline fp8x8 x3d_add_fp8x8(fp8x8 a, fp8x8 b) {
#ifdef X3D_CHECK_OVERFLOW
  x3d_errorif(a + b != (long)a + b, "%d, %d", a, b);
#endif

  return a + b;
}

