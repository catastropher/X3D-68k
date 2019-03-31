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

#include "RenderingUtil.hpp"
#include "geo/BoundBox.hpp"
#include "geo/Ray3.hpp"

void renderBoundBox(BoundBoxfp& boundBox, X_RenderContext& renderContext, X_Color color)
{
    Vec3fp x = Vec3fp(boundBox.v[1].x - boundBox.v[0].x, 0, 0);
    Vec3fp y = Vec3fp(0, boundBox.v[1].y - boundBox.v[0].y, 0);
    Vec3fp z = Vec3fp(0, 0, boundBox.v[1].z - boundBox.v[0].z);

    Vec3fp topLeft = boundBox.v[0];

    Vec3fp v[2][4] =
    {
        {
            topLeft,
            topLeft + z,
            topLeft + z + x,
            topLeft + x
        },
        {
            topLeft + y,
            topLeft + z + y,
            topLeft + z + x + y,
            topLeft + x + y
        }
    };

    for(int i = 0; i < 4; ++i)
    {
        int nextVertexIndex = (i + 1) & 3;

        Ray3 top(v[0][i], v[0][nextVertexIndex]);
        top.render(renderContext, color);

        Ray3 bottom(v[1][i], v[1][nextVertexIndex]);
        bottom.render(renderContext, color);

        Ray3 side = Ray3(v[0][i], v[1][i]);
        side.render(renderContext, color);
    }
}
