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

#include "X3D_common.h"
#include "render/X3D_texture.h"

///////////////////////////////////////////////////////////////////////////////
/// Loads a texture from a file (stored in a bmp file).
///
/// @param tex  - texture struct to load into
/// @param file - name of file to load texture from
///
/// @return Whether the texture was successfully loaded.
/// @note   Only try to load texture that have size which are powers of 2!
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_texture_load_from_file(X3D_Texture* tex, const char* file) {
  return x3d_platform_screen_load_texture(tex, file);
}

///////////////////////////////////////////////////////////////////////////////
/// Blits a texture onto the screen as a 2D image.
///
/// @param tex  - texture
/// @param x    - x coordinate of top right corner of where the texture goes
/// @param y    - y coordinate of top right corner of where the texture goes
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_texture_blit(X3D_Texture* tex, uint16 x, uint16 y) {
  uint16 i, d;
  
  for(i = 0; i < tex->h; ++i) {
    for(d = 0; d < tex->w; ++d) {
      x3d_screen_draw_pix(d + x, i + y, x3d_texture_get_texel(tex, d, i));
    }
  }
}


void x3d_texture_to_array(X3D_Texture* texture, FILE* file, const char* name) {
  fprintf(file, "uint8 %s_data[] = {\n  %d,\n  %d", name, texture->w, texture->h);
  
  uint32 i;
  for(i = 0; i < (uint32)texture->w * texture->h; ++i) {
    X3D_Color c = texture->texel[i];
    uint8 r, g, b;
    x3d_color_to_rgb(c, &r, &g, &b);
    
    fprintf(file, ",\n  %d,\n  %d,  \n  %d", r, g, b);
  }
  
  fprintf(file, "\n};\n\n");
}

void x3d_fix_texture(X3D_Texture* tex);

void x3d_texture_from_array(X3D_Texture* dest, uint8* data) {
  dest->w = data[0];
  dest->h = data[1];
  dest->texel = malloc(sizeof(X3D_Color) * dest->w * dest->h);
  dest->mask = dest->w - 1;
  
  x3d_log(X3D_INFO, "width: %d", dest->w);
  x3d_log(X3D_INFO, "height: %d", dest->h);
  
  uint32 i;
  for(i = 0; i < (uint32)dest->w * dest->h; ++i) {
    dest->texel[i] = x3d_rgb_to_color(data[2 + i * 3], data[2 + i * 3 + 1], data[2 + i * 3 + 2]);
    
  }
  
#ifdef __nspire__
  x3d_fix_texture(dest);
#endif
}


