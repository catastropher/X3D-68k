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

/// Approximation for the slope of a vertical line (infinity).
#define X_VERTICAL_LINE_SLOPE 0x7FFFFFFF

x_fp16x16 x_sin(x_fp16x16 angle);

////////////////////////////////////////////////////////////////////////////////
/// Calculates the cosine of an angle using a lookup table.
///
/// @param angle    - angle in base 256
///
/// @return cos(angle) as an x_fp16x16
////////////////////////////////////////////////////////////////////////////////
static inline x_fp16x16 x_cos(x_fp16x16 angle)
{
    return x_sin(X_ANG_90 - angle);
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the tangent of an angle using a lookup table.
///
/// @param angle    - angle in base 256
///
/// @return tan(angle) as an x_fp16x16
///
/// @note This will return @ref X_VERTICAL_LINE_SLOPE if angle == X_ANG_90 ||
///     angle == X_ANG_270.
////////////////////////////////////////////////////////////////////////////////
static inline x_fp16x16 x_tan(x_fp16x16 angle)
{
    // Prevent division by 0
    if(angle == X_ANG_90 || angle == X_ANG_270)
        return X_VERTICAL_LINE_SLOPE;

    return x_fp16x16_div(x_sin(angle), x_cos(angle));
}

