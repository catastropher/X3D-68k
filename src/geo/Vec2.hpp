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

////////////////////////////////////////////////////////////////////////////////
/// A 2D vector or vertex.
////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct Vec2Generic
{
    Vec2Generic(T x_, T y_) : x(x_), y(y_) { }
    Vec2Generic() { }
    
    Vec2Generic operator-(const Vec2Generic& v) const
    {
        return Vec2Generic(x - v.x, y - v.y);
    }

    Vec2Generic operator+(const Vec2Generic& v) const
    {
        return Vec2Generic(x + v.x, y + v.y);
    }

    bool operator==(const Vec2Generic& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    void print() const
    {
        printf("%f %f\n", convert<float>(x), convert<float>(y));
    }
    
    T x;
    T y;
};

using Vec2_fp16x16 = Vec2Generic<int>;
using Vec2 = Vec2Generic<int>;
using Vec2i = Vec2Generic<int>;
using Vec2fp = Vec2Generic<fp>;

static inline Vec2fp MakeVec2fp(const Vec2_fp16x16& v)
{
    return Vec2fp(fp(v.x), fp(v.y));
}
