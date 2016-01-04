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

