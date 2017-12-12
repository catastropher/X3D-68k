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

#include "X_Texture.h"
#include "system/X_File.h"
#include "error/X_log.h"
#include "util/X_util.h"

_Bool x_texture_save_to_xtex_file(const X_Texture* tex, const char* fileName)
{
    X_File file;
    if(!x_file_open_writing(&file, fileName))
        return 0;
    
    x_file_write_buf(&file, 4, "XTEX");
    x_file_write_le_int16(&file, x_texture_w(tex));
    x_file_write_le_int16(&file, x_texture_h(tex));
    x_file_write_buf(&file, x_texture_total_texels(tex), tex->texels);
    
    x_file_close(&file);
    
    return 1;
}

_Bool x_texture_load_from_xtex_file(X_Texture* tex, const char* fileName)
{
    X_File file;
    if(!x_file_open_reading(&file, fileName))
        return 0;
    
    char signature[5];
    x_file_read_fixed_length_str(&file, 4, signature);
    
    if(strcmp(signature, "XTEX") != 0)
    {
        x_log_error("File %s has bad XTEX header", fileName);
        return 0;
    }
    
    int w = x_file_read_le_int16(&file);
    int h = x_file_read_le_int16(&file);
    
    x_texture_init(tex, w, h);
    x_file_read_buf(&file, x_texture_total_texels(tex), tex->texels);
    
    x_file_close(&file);
    
    return 1;
}

void x_texture_clamp_vec2(const X_Texture* tex, X_Vec2* v)
{
    v->x = X_MAX(v->x, 0);
    v->x = X_MIN(v->x, tex->w - 1);
    
    v->y = X_MAX(v->y, 0);
    v->y = X_MIN(v->y, tex->h - 1);
}

////////////////////////////////////////////////////////////////////////////////
/// Draws a line of the given color into a canvas.
/// @note This function ignores the z-buffer
////////////////////////////////////////////////////////////////////////////////
void x_texture_draw_line(X_Texture* tex, X_Vec2 start, X_Vec2 end, X_Color color)
{
    x_texture_clamp_vec2(tex, &start);
    x_texture_clamp_vec2(tex, &end);
    
    int dx = abs(end.x - start.x);
    int sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y);
    int sy = start.y < end.y ? 1 : -1; 
    int err = (dx > dy ? dx : -dy) / 2;
    X_Vec2 pos = start;
    
    while(1)
    {
        x_texture_set_texel(tex, pos.x, pos.y, color);
        
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

