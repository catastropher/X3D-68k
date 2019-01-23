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

#include "FixedPoint.hpp"
#include "Angle.hpp"

/// Approximation for the slope of a vertical line (infinity).
#define X_VERTICAL_LINE_SLOPE 0x7FFFFFFF

fp x_sin(fp angle);
fp x_asin(fp val);
fp x_acos(fp val);
fp x_atan2(fp y, fp x);

////////////////////////////////////////////////////////////////////////////////
/// Calculates the cosine of an angle using a lookup table.
///
/// @param angle    - angle in base 256
///
/// @return cos(angle) as an x_fp16x16
////////////////////////////////////////////////////////////////////////////////
static inline fp x_cos(fp angle)
{
    // FIXME: this will break once the angle constants are converted
    return x_sin(fp(X_ANG_90 - angle.internalValue()));
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
static inline fp x_tan(fp angle)
{
    // Prevent division by 0
    if(angle == fp(X_ANG_90) || angle == fp(X_ANG_270))
        return X_VERTICAL_LINE_SLOPE;

    return x_sin(angle) / x_cos(angle);
}

static inline fp angleToRadians(fp angle)
{
    const fp PI = fp::fromFloat(3.14159);

    return angle * PI / 128;
}

static inline fp radiansToAngle(fp radians)
{
    const fp PI = fp::fromFloat(3.14159);

    return 128 * radians / PI;
}

static inline void adjustAngle(fp& angle)
{
    fp unitsInCircle = fp::fromInt(256);

    while(angle >= unitsInCircle)
    {
        angle -= unitsInCircle;
    }

    while(angle < 0)
    {
        angle += unitsInCircle;
    }
}

