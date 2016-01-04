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

/// The number of bits in an X3D normal (as in surface normal)
#define X3D_NORMAL_BITS 15

///////////////////////////////////////////////////////////////////////////////
/// A 3D vector with int16 components.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Vex3D_int16 {
  int16 x;
  int16 y;
  int16 z;
} X3D_Vex3D_int16;

///////////////////////////////////////////////////////////////////////////////
/// A 3D vector with uint8 components.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Vex3D_uint8 {
  int16 x;
  int16 y;
  int16 z;
} X3D_Vex3D_uint8;

typedef X3D_Vex3D_int16 X3D_Vex3D;
typedef X3D_Vex3D_int16 X3D_Vex3D_fp0x16;

typedef X3D_Vex3D_uint8 X3D_Vex3D_angle256;

