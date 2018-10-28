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

#include "Fp.hpp"

namespace X3D
{
    class Angle
    {
    public:
        static constexpr fp PI = fp(3.1415926 * 65536);

        Angle()
        {

        }

        constexpr Angle(fp angle_)
            : angle(angle_)
        {

        }

        constexpr fp toDegrees()
        {
            reduce();

            return angle * 360 / 256;
        }

        constexpr fp toRadians()
        {
            reduce();

            return angle * PI / 128;
        }

        fp sin() const;
        fp cos() const;
        fp tan() const;

        static Angle atan2(fp y, fp x);
        static Angle acos(fp val);
        static Angle asin(fp val);

        static constexpr Angle fromDegrees(fp degrees)
        {
            return Angle(degrees * 256 / 360);
        }

        static constexpr Angle fromRadians(fp radians)
        {
            return Angle(128 * radians / PI);
        }

        constexpr Angle operator+(const Angle& ang) const
        {
            return Angle(angle + ang.angle);
        }

        constexpr Angle operator-(const Angle& ang) const
        {
            return Angle(angle - ang.angle);
        }

        constexpr bool operator==(const Angle& ang) const
        {
            return angle == ang.angle;
        }

        constexpr Angle operator-() const
        {
            return Angle(-angle);
        }

        constexpr fp toFp()
        {
            reduce();

            return angle;
        }

    private:
        // Reduces an angle to be in (0, 256]
        constexpr void reduce()
        {
            int angleAsInt = angle.internalValue();

            if(angleAsInt < 0)
            {
                // Add enough of a large multiple of 256 to make this positive (hopefully)
                angleAsInt += (1 << 30);
            }

            angle = fp(angleAsInt & 0x00FFFFFF);
        }

        fp angle;
    };
}

