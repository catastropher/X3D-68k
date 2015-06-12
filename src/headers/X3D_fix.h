#pragma once
#define X3D_CHECK_OVERFLOW

#include "X3D_config.h"
#include "x3d_error.h"

#ifdef __TIGCC__

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

#else

#include <stdint.h>

/// An 8-bit signed integer
typedef int8_t int8;

/// An 8-bit unsigned integer
typedef uint8_t uint8;

/// A 16-bit signed integer
typedef int16_t int16;

/// A 16-bit unsigned integer
typedef uint16_t uint16;

/// A 32-bit signed integer
typedef int32_t int32;

/// A 32-bit unsigned integer
typedef uint32_t uint32;

/// A 64-bit signed integer
typedef int64_t int64;

/// A 64-bit unsigned integer
typedef uint64_t uint64;

#endif

/// An 8.8 fixed point number
typedef int16 fp8x8;

/// A 16.16 fixed point number
typedef int32 fp16x16;

/// A 0.16 fixed point number
typedef int16 fp0x16;


/// Returns the sign of a value
/// @todo Look into implementing as (_val >> ((sizeof(_val) * 8 - 1)
#define X3D_SIGNOF(_val) (_val < 0 ? -1 : (_val > 0 ? 1 : 0))

//=============================================================================
// Inline functions
//=============================================================================

/**
 * Adds two int8 numbers, possibly checking for overflow.
 *
 * @param a   - first fixed point number
 * @param b   - second fixed point number
 *
 * @return sum of a and b as an fp8x8
 *
 * @note If @ref X3D_CHECK_OVERFLOW is defined, this will throw an error, if a + b overflows/underflows.
 */
static inline int8 add_int8_overflow(int16 a, int16 b) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_INT8, a);
  X3D_PARAM(PARAM_INT8, b);

#ifdef X3D_CHECK_OVERFLOW
  x3d_errorif((int16)a + b != (int8)(a + b), "int8 overflow");
#endif

  return a + b;
}

/**
* Adds two uint8 numbers, possibly checking for overflow.
*
* @param a   - first fixed point number
* @param b   - second fixed point number
*
* @return sum of a and b as an fp8x8
*
* @note If @ref X3D_CHECK_OVERFLOW is defined, this will throw an error, if a + b overflows/underflows.
*/
static inline int8 add_uint8_overflow(int16 a, int16 b) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_UINT8, a);
  X3D_PARAM(PARAM_UINT8, b);

#ifdef X3D_CHECK_OVERFLOW
  x3d_errorif((uint16)a + b != (uint8)(a + b), "uint8 overflow");
#endif

  return a + b;
}

/**
* Adds two int16 numbers, possibly checking for overflow.
*
* @param a   - first fixed point number
* @param b   - second fixed point number
*
* @return sum of a and b as an fp8x8
*
* @note If @ref X3D_CHECK_OVERFLOW is defined, this will throw an error, if a + b overflows/underflows.
*/
static inline int16 add_int16_overflow(int16 a, int16 b) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_INT16, a);
  X3D_PARAM(PARAM_INT16, b);

#ifdef X3D_CHECK_OVERFLOW
  x3d_errorif((int32)a + b != (int16)(a + b), "int16 overflow");
#endif

  return a + b;
}

/**
* Adds two uint16 numbers, possibly checking for overflow.
*
* @param a   - first fixed point number
* @param b   - second fixed point number
*
* @return sum of a and b as an fp8x8
*
* @note If @ref X3D_CHECK_OVERFLOW is defined, this will throw an error, if a + b overflows/underflows.
*/
static inline uint16 add_uint16_overflow(uint16 a, uint16 b) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_UINT16, a);
  X3D_PARAM(PARAM_UINT16, b);

#ifdef X3D_CHECK_OVERFLOW
  x3d_errorif((uint32)a + b != (uint16)(a + b), "uint16 overflow");
#endif

  return a + b;
}

/**
* Adds two int32 numbers, possibly checking for overflow.
*
* @param a   - first fixed point number
* @param b   - second fixed point number
*
* @return sum of a and b as an fp8x8
*
* @note If @ref X3D_CHECK_OVERFLOW is defined, this will throw an error, if a + b overflows/underflows.
*/
static inline int32 add_int32_overflow(int32 a, int32 b) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_INT32, a);
  X3D_PARAM(PARAM_INT32, b);

#ifdef X3D_CHECK_OVERFLOW
  x3d_errorif((int64)a + b != (int32)(a + b), "int32 overflow");
#endif

  return a + b;
}

/**
* Adds two uint32 numbers, possibly checking for overflow.
*
* @param a   - first fixed point number
* @param b   - second fixed point number
*
* @return sum of a and b as an fp8x8
*
* @note If @ref X3D_CHECK_OVERFLOW is defined, this will throw an error, if a + b overflows/underflows.
*/
static inline uint32 add_uint32_overflow(uint32 a, uint32 b) {
  X3D_STACK_TRACE;

  X3D_PARAM(PARAM_UINT32, a);
  X3D_PARAM(PARAM_UINT32, b);

#ifdef X3D_CHECK_OVERFLOW
  x3d_errorif((uint64)a + b != (uint32)(a + b), "uint32 overflow");
#endif

  return a + b;
}


