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

#include "X3D_common.h"
#include "X3D_texture.h"

typedef struct X3D_Font {
  int16 glyph_width;
  int16 glyph_height;
  
  int16 glyph_offset_x;
  int16 glyph_offset_y;
  
  int16 font_space_x;
  int16 font_space_y;
  
  int16 font_offset_x;
  int16 font_offset_y;
  
  int16 font_rows;
  int16 font_cols;
  X3D_Texture tex;
} X3D_Font;

_Bool x3d_font_load(X3D_Font* font, const char* file_name);
void x3d_font_draw_char(X3D_Font* font, unsigned char c, int16 x, int16 y);
void x3d_font_draw_str(X3D_Font* font, const char* str, int16 x, int16 y);
int16 x3d_font_str_height(X3D_Font* font, const char* str);

