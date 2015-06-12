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

#pragma once

#include "X3D_config.h"
#include "X3D_fix.h"

/// @todo document
#define ANG_0 0
#define ANG_30 21
#define ANG_45 32
#define ANG_60 42
#define ANG_90 64
#define ANG_180 128
#define ANG_270 192
#define ANG_360 256

/// @todo document
#define VERTICAL_LINE_SLOPE INT16_MAX

extern const int16 sintab[256];

// Given the input angle in DEG256, this returns the fixed-point sine of it
// The number is in 0:15 format
/// @todo update documentation
/// @todo add tests
static inline fp0x16 x3d_sinfp(angle256 angle) {
  return sintab[(uint16)angle];
}

// Given the input angle in DEG256, this returns the fixed-point cosine of it
// The number is in 0:15 format
/// @todo update documentation
/// @todo add tests
static inline fp0x16 x3d_cosfp(angle256 angle) {
  // We exploit the fact that cos(x) = sin(90 - x)
  return x3d_sinfp(ANG_90 - angle);
}

// Given the input angle in DEG256, this returns the fixed-point cosine of it
// The number is in 8:8 format
/// @todo update documentation
/// @todo add tests
static inline fp8x8 x3d_tanfp(angle256 angle) {
  // Prevent division by 0
  if(angle == ANG_90 || angle == ANG_180)
    return VERTICAL_LINE_SLOPE;

  return div_fp0x16_by_fp0x16(x3d_sinfp(angle), x3d_cosfp(angle));
}

