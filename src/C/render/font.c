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

#include "render/X3D_font.h"
#include "render/X3D_texture.h"

_Bool x3d_font_load(X3D_Font* font, const char* file_name) {
  return x3d_texture_load_from_file(&font->tex, file_name);
}

void x3d_font_draw_char(X3D_Font* font, unsigned char c, int16 x, int16 y) {
  int16 row = c >> 4;
  int16 col = c & 0x0F;
  
  int16 font_x = font->font_offset_x + col * font->font_space_x + font->glyph_offset_x;
  int16 font_y = font->font_offset_y + row * font->font_space_y + font->glyph_offset_y;
  
  int16 i, d;
  for(i = 0; i <= font->glyph_height; ++i) {
    for(d = 0; d <= font->glyph_width; ++d) {
      //x3d_log(X3D_INFO, "tex pos: %d, %d", d + font_x, i + font_y);
      
      x3d_screen_draw_pix(d + x, i + y, x3d_texture_get_texel(&font->tex, d + font_x, i + font_y));
    }
  }
}

void x3d_font_draw_str(X3D_Font* font, const char* str, int16 x, int16 y) {
  int16 start_x = x;
  
  while(*str) {
    if(*str == '\n') {
      x = start_x;
      y += font->glyph_height;
    }
    else {
      x3d_font_draw_char(font, *str, x, y);
      x += font->glyph_width;
    }
    
    ++str;
  }
}

int16 x3d_font_str_height(X3D_Font* font, const char* str) {
  int16 height = font->glyph_height;
  
  while(*str) {
    if(*str == '\n') {
      height += font->glyph_height;
    }
    
    ++str;
  }
  
  return height;
}

