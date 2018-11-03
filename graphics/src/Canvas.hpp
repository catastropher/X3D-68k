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

#include <X3D-math.hpp>

#include "Texture.hpp"

namespace X3D
{
    class Canvas
    {
    public:
        Canvas(Texture* texture_)
            : texture(texture_),
            maxX(fp::fromInt(texture->getW() - 1)),
            maxY(fp::fromInt(texture->getH() - 1))
        {

        }

        void clamp(Vec2& v)
        {
            v.x = X3D::clamp(v.x, fp(0), maxX);
            v.y = X3D::clamp(v.y, fp(0), maxY);
        }

        void drawLine(Vec2 start, Vec2 end, Color color);

    private:
        Texture* texture;
        fp maxX;
        fp maxY;
    };
}

