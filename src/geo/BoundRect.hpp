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

#include "Vec2.hpp"
#include "util/Util.hpp"

struct BoundRect
{
    void clear()
    {
        v[0].x = v[0].y = maxValue<fp>();
        v[1].x = v[1].y = minValue<fp>();
    }

    void addPoint(Vec2fp point)
    {
        if(point.x < v[0].x)
        {
            v[0].x = point.x;
        }
        else if(point.x > v[1].x)
        {
            v[1].x = point.x;
        }

        if(point.y < v[0].y)
        {
            v[0].y = point.y;
        }
        else if(point.y > v[1].y)
        {
            v[1].y = point.y;
        }
    }

    bool pointInside(Vec2fp point) const
    {
        return inRange(point.x, v[0].x, v[1].x)
            && inRange(point.y, v[0].y, v[1].y);
    }

    Vec2fp v[2];
};

