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
#include "X_Font.h"

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

void x_texture_blit_texture(X_Texture* canvas, const X_Texture* tex, X_Vec2 pos)
{
    int endX = X_MIN(pos.x + x_texture_w(tex), canvas->w);
    int endY = X_MIN(pos.y + x_texture_h(tex), canvas->h);
    
    for(int y = pos.y; y < endY; ++y)
    {
        for(int x = pos.x; x < endX; ++x)
        {
            x_texture_set_texel(canvas, x, y, x_texture_get_texel(tex, x - pos.x, y - pos.y));
        }
    }
}

void x_texture_draw_char(X_Texture* canvas, unsigned char c, const X_Font* font, X_Vec2 pos)
{
    const X_Color* charPixels = x_font_get_character_pixels(font, c);

    X_Vec2 clippedTopLeft = x_vec2_make
    (
        X_MAX(0, pos.x) - pos.x,
        X_MAX(0, pos.y) - pos.y
    );
    
    X_Vec2 clippedBottomRight = x_vec2_make
    (
        X_MIN(canvas->w, pos.x + font->charW) - pos.x,
        X_MIN(canvas->h, pos.y + font->charH) - pos.y
    );
    
    for(int i = clippedTopLeft.y; i < clippedBottomRight.y; ++i)
        for(int j = clippedTopLeft.x; j < clippedBottomRight.x; ++j)
            x_texture_set_texel(canvas, pos.x + j, pos.y + i, *charPixels++);
}

void x_texture_draw_str(X_Texture* canvas, const char* str, const X_Font* font, X_Vec2 pos)
{
    X_Vec2 currentPos = pos;
    
    while(*str)
    {
        if(*str == '\n')
        {
            currentPos.x = pos.x;
            currentPos.y += font->charH;
        }
        else {
            x_texture_draw_char(canvas, *str, font, currentPos);
            currentPos.x += font->charW;
        }
        
        ++str;
    }
}

void x_texture_fill_rect(X_Texture* canvas, X_Vec2 topLeft, X_Vec2 bottomRight, X_Color color)
{
    x_texture_clamp_vec2(canvas, &topLeft);
    x_texture_clamp_vec2(canvas, &bottomRight);
    
    for(int y = topLeft.y; y <= bottomRight.y; ++y)
    {
        for(int x = topLeft.x; x <= bottomRight.x; ++x)
        {
            x_texture_set_texel(canvas, x, y, color);
        }
    }
}

