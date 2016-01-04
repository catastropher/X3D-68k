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

#include <stdint.h>
#include <stdarg.h>

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

enum {
  X3D_FALSE = 0,
  X3D_TRUE = 1
};

///////////////////////////////////////////////////////////////////////////////
/// Checks a series of int32's and checks if there is overflow (or underflow).
///
/// @param total  - total number of int32's
/// @param ...    - int32's to add
///
/// @return Whether there was an overflow or underflow in adding them.
///////////////////////////////////////////////////////////////////////////////
static inline _Bool x3d_addi32_check_overflow(int16 total, ...) {
  int32 sum = 0;
  va_list list;
  
  va_start(list, total);
  
  while(total-- > 0) {
    int32 num = va_arg(list, int32);
    
    // Overflow/underflow can only happen if the signs are the same before
    // adding, and become the opposite sign after adding
    if((sum < 0 && num < 0 && sum + num > 0) ||
        (sum > 0 && num > 0 && sum + num < 0)) {
      
      return X3D_TRUE;
    }
  }
  
  return X3D_FALSE;
}

/// @todo fix formatting

/**
* Rotates the lower byte of a uint16.
*
* @param i - input
*
* @return The lower byte
*/
static inline uint8 x3d_uint16_lower(uint16 i) {
  return i & 0xFF;
}

/**
* Rotates the upper byte of a uint16.
*
* @param i - input
*
* @return The upper byte
*/
static inline uint8 x3d_uint16_upper(uint16 i) {
  return i >> 8;
}