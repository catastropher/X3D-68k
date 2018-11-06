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
#include "Limits.hpp"

namespace X3D
{
    template<typename T>
    struct Vec2Template
    {
        constexpr Vec2Template(T x_, T y_)
            : x(x_),
            y(y_)
        {

        }

        template<typename U>
        constexpr Vec2Template<U> toVec2()
        {
            return Vec2Template<U>(
                convert<U>(x),
                convert<U>(y));
        }

        bool operator==(const Vec2Template& v) const
        {
            return x == v.x && y == v.y;
        }

        T x;
        T y;
    };

    using Vec2 = Vec2Template<fp>;
    using Vec2i = Vec2Template<int>;

    template<typename T>
    constexpr inline Vec2Template<T> maxValue()
    {
        return Vec2Template<T>(
            maxValue<T>(),
            maxValue<T>());
    }

    template<typename T>
    constexpr inline Vec2Template<T> minValue()
    {
        return Vec2Template<T>(
            minValue<T>(),
            minValue<T>());
    }
}

