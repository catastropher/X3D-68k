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

///< Quick angle360 to angle256 conversion constants
enum {
  ANG_0 = 0,
  ANG_30 = 21,
  ANG_45 = 32,
  ANG_60 = 42,
  ANG_90 = 64,
  ANG_180 = 128,
  ANG_270 = 192,
  ANG_360 = 256
};

///< The slope of a vertical line in fp8x8 format (faking infinity)
#define VERTICAL_LINE_SLOPE INT16_MAX

extern const fp0x16 sintab[256];

/**
* Calculates the sine of an angle.
*
* @param angle - the angle as an angle256
*
* @return The sine of the angle in fp0x16 format
*/
static inline fp0x16 x3d_sinfp(angle256 angle) {
  return sintab[(uint16)angle];
}

/**
* Calculates the cosine of an angle.
*
* @param angle - the angle as an angle256
*
* @return The cosine of the angle in fp0x16 format
*/
static inline fp0x16 x3d_cosfp(angle256 angle) {
  // We exploit the fact that cos(x) = sin(90 - x)
  return x3d_sinfp(ANG_90 - angle);
}

/**
* Calculates the tangent of an angle.
*
* @param angle - the angle as an angle256
*
* @return The tangent of the angle in fp8x8 format
* @note If angle is ANG_90 or ANG_270, this returns @ref VERTICAL_LINE_SLOPE
*/
static inline fp8x8 x3d_tanfp(angle256 angle) {
  // Prevent division by 0
  if(angle == ANG_90 || angle == ANG_270)
    return VERTICAL_LINE_SLOPE;

  return div_fp0x16_by_fp0x16(x3d_sinfp(angle), x3d_cosfp(angle));
}

