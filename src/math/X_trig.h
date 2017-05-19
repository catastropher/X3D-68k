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

#include "X_fix.h"
#include "X_angle.h"

/// @todo Document this file

enum {
    X_ANG_0 = 0,
    X_ANG_30 = 21,
    X_ANG_45 = 32,
    X_ANG_60 = 42,
    X_ANG_90 = 64,
    X_ANG_180 = 128,
    X_ANG_270 = 192,
    X_ANG_360 = 256
};

#define X_VERTICAL_LINE_SLOPE 0x7FFFFFFF

fp16x16 x_sin(angle256 angle);

static inline fp16x16 x_cos(angle256 angle)
{
    return x_sin(X_ANG_90 - angle);
}

static inline fp16x16 x_tan(angle256 angle)
{
  // Prevent division by 0
  if(angle == X_ANG_90 || angle == X_ANG_270)
    return X_VERTICAL_LINE_SLOPE;

  return x_fix_div_fp16x16(x_sin(angle), x_cos(angle));
}

