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

#include "X_Canvas.h"
#include "util/X_util.h"

void x_canvas_fill_rect(X_Canvas* canvas, X_Vec2 topLeft, X_Vec2 bottomRight, X_Color color)
{
    x_texture_clamp_vec2(&canvas->tex, &topLeft);
    x_texture_clamp_vec2(&canvas->tex, &bottomRight);
    
    for(int y = topLeft.y; y <= bottomRight.y; ++y)
    {
        for(int x = topLeft.x; x <= bottomRight.x; ++x)
        {
            x_texture_set_texel(&canvas->tex, x, y, color);
        }
    }
}

