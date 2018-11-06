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
#include "Limits.hpp"

namespace X3D
{
    template<typename T>
    struct BoundRectTemplate
    {
        using Point = Vec2Template<T>;

        BoundRectTemplate()
            : topLeft(maxValue<Point>()),
            bottomRight(minValue<Point>())
        {

        }

        BoundRectTemplate(const Point& topLeft_, const Point& bottomRight_)
            : topLeft(topLeft_),
            bottomRight(bottomRight_)
        {

        }

        void addPoint(const Point& p)
        {
            if(p.x < topLeft.x)
            {
                topLeft.x = p.x;
            }
            
            if(p.x > bottomRight.x)
            {
                bottomRight.x = p.x;
            }

            if(p.y < topLeft.x)
            {
                topLeft.y = p.y;
            }

            if(p.y > bottomRight.y)
            {
                bottomRight.y = p.y;
            }
        }

        void merge(const BoundRectTemplate& rect)
        {
            addPoint(rect.topLeft);
            addPoint(rect.bottomRight);
        }

        bool doesIntersect(const BoundRectTemplate& rect) const
        {
            if(rect.bottomRight.x < topLeft.x || rect.topLeft.x > bottomRight.x)
            {
                return false;
            }

            if(rect.bottomRight.y < topLeft.y || rect.topLeft.y > bottomRight.y)
            {
                return false;
            }

            return true;
        }

        void intersect(BoundRectTemplate& rect, BoundRectTemplate& dest)
        {
            
        }

        Point topLeft;
        Point bottomRight;
    };

    using BoundRect = BoundRectTemplate<fp>;
    using BoundRecti = BoundRectTemplate<int>;
}

