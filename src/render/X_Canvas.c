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

////////////////////////////////////////////////////////////////////////////////
/// Draws a line of the given color into a canvas.
/// @note This function ignores the z-buffer
////////////////////////////////////////////////////////////////////////////////
void x_canvas_draw_line(X_Canvas* canvas, X_Vec2 start, X_Vec2 end, X_Color color)
{
    int dx = abs(end.x - start.x);
    int sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y);
    int sy = start.y < end.y ? 1 : -1; 
    int err = (dx > dy ? dx : -dy) / 2;
    X_Vec2 pos = start;
    
    while(1)
    {
        x_texture_set_texel(&canvas->tex, pos.x, pos.y, color);
        
        if(x_vec2_equal(&pos, &end))
            break;
        
        int old_err = err;
        if (old_err > -dx)
        {
            err -= dy;
            pos.x += sx;
        }
        
        if (old_err < dy)
        {
            err += dx;
            pos.y += sy;
        }
    }
}

