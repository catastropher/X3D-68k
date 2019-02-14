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

#include "math/FixedPoint.hpp"
#include "Vec3.hpp"

template<typename T>
struct Vec4Template
{
    Vec4Template() { }
    constexpr Vec4Template(T x_, T y_, T z_, T w_)
        : x(x_),
        y(y_),
        z(z_),
        w(w_)
    {

    }

    constexpr Vec4Template(const Vec3Template<T>& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = 0;
    }

    constexpr Vec3Template<T> toVec3() const
    {
        return Vec3Template<T>(x, y, z);
    }

    T x;
    T y;
    T z;
    T w;
};

using Vec4 = Vec4Template<fp>;

