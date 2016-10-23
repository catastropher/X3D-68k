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
#include "render/X3D_palette.h"

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
  
  uint32 i, d;
  
  for(i = 0; i < texture->h; ++i) {
    for(d = 0; d < texture->w; ++d) {
      X3D_Color c = x3d_texture_get_texel(texture, d, i);
      uint8 r, g, b;
      x3d_color_to_rgb(c, &r, &g, &b);
      
      fprintf(file, ",\n  %d,\n  %d,  \n  %d", r, g, b);
    }
  }
  
  fprintf(file, "\n};\n\n");
}

void x3d_fix_texture(X3D_Texture* tex);

uint8 x3d_texture_get_color_index(X3D_Texture* tex, X3D_Color c) {
  uint16 i;
  for(i = 0; i < tex->total_c; ++i) {
    if(tex->color_tab[i] == c) {
      return i;
    }
  }
  
  tex->color_tab[tex->total_c] = c;
  
  return tex->total_c++;
}

static int16 color_diff(int16 r1, int16 g1, int16 b1, int16 r2, int16 g2, int16 b2) {
  return abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2);
}

void x3d_texture_replace_most_similar_color(X3D_Texture* tex, X3D_Color* texel) {
  uint16 i, d;
  int16 min_diff = 0x7FFF;
  int16 min_a = -1;
  int16 min_b = -1;
  
  for(i = 0; i < tex->total_c; ++i) {
    for(d = i + 1; d < tex->total_c; ++d) {
      uint8 r1, g1, b1;
      x3d_color_to_rgb(tex->color_tab[i], &r1, &g1, &b1);
      
      uint8 r2, g2, b2;
      x3d_color_to_rgb(tex->color_tab[d], &r2, &g2, &b2);
      
      int16 diff = color_diff(r1, g1, b1, r2, g2, b2);
      
      if(diff < min_diff) {
        min_diff = diff;
        min_a = i;
        min_b = d;
      }
    }
  }
  
  X3D_Color find = tex->color_tab[min_a];
  X3D_Color replace = tex->color_tab[min_b];
  
  for(i = 0; i < tex->w * tex->h; ++i) {
    if(texel[i] == find) {
      texel[i] = replace;
    }
  }
  
  for(i = min_a; i < tex->total_c - 1; ++i) {
    tex->color_tab[i] = tex->color_tab[i + 1];
  }
  
  --tex->total_c;
}

void x3d_texture_pack_4bit(X3D_Texture* tex, X3D_Color* texel) {
  tex->flags |= X3D_TEXTURE_4BIT;
  tex->texel.small = malloc(tex->w * tex->h / 2);

  x3d_log(X3D_INFO, "Packed into 4 bit texture");
  
  uint16 i;
  for(i = 0; i < tex->w * tex->h; i += 2) {
    tex->texel.small[i / 2] = (x3d_texture_get_color_index(tex, texel[i]) << 4) |
      x3d_texture_get_color_index(tex, texel[i + 1]);
  }
}

void x3d_texture_pack_8bit(X3D_Texture* tex, X3D_Color* texel) {
  tex->texel.large = malloc((uint32)tex->w * tex->h);
  
  uint32 i;
  for(i = 0; i < (uint32)tex->w * tex->h; ++i) {
    tex->texel.large[i] = x3d_color_to_colorindex(texel[i]); //x3d_texture_get_color_index(tex, texel[i]);
  }
}

void x3d_texture_resize_to_64(X3D_Color* texel) {
  uint16 i, d;
  uint16 pos = 0;
  
  for(i = 0; i < 128; i += 2) {
    for(d = 0; d < 128; d += 2) {
      texel[pos++] = texel[i * 128 + d];
    }
  }
}


void x3d_texture_from_array(X3D_Texture* dest, uint8* data) {
  if(data[0] == 0) {
    dest->w = ((uint16)data[1] << 8) + data[2];
    dest->h = ((uint16)data[3] << 8) + data[4];
    
    x3d_log(X3D_INFO, "Width: %d, %d", dest->w, dest->h);
    data += 3;
  }
  else {
    dest->w = data[0];
    dest->h = data[1];
  }
  
  X3D_Color* texel = malloc(sizeof(X3D_Color) * dest->w * dest->h);
  dest->color_tab = malloc(256 * sizeof(X3D_Color));
  dest->total_c = 0;
  dest->mask = dest->w - 1;

  if(dest->w == 32 || dest->w == 64 || dest->w == 128) {
    dest->flags |= X3D_TEXTURE_REPEAT;
  }
  
  //x3d_log(X3D_INFO, "width: %d", dest->w);
  //x3d_log(X3D_INFO, "height: %d", dest->h);
  
  uint32 i;
  for(i = 0; i < (uint32)dest->w * dest->h; ++i) {
    texel[i] = x3d_rgb_to_color(data[2 + i * 3], data[2 + i * 3 + 1], data[2 + i * 3 + 2]);
    
    //x3d_texture_get_color_index(dest, texel[i]);
  }
  
  if(dest->w == 128) {
    dest->w = 64;
    dest->h = 64;
    dest->mask = dest->w - 1;
    
    x3d_texture_resize_to_64(texel);
  }
  
  if(dest->total_c <= 20 && 0) {
    while(dest->total_c > 16) {
      x3d_texture_replace_most_similar_color(dest, texel);
    }
    
    x3d_texture_pack_4bit(dest, texel);
  }
  else {
    x3d_texture_pack_8bit(dest, texel);
  }
  
  free(texel);
  
  x3d_log(X3D_INFO, "Texture has %d colors", dest->total_c);
  
#ifdef __nspire__
  x3d_fix_texture(dest);
#endif
}

void x3d_texture_init(X3D_Texture* tex, uint16 w, uint16 h) {
    tex->w = w;
    tex->h = h;
    tex->texel.large = malloc(sizeof(X3D_ColorIndex) * w * h);
}

void x3d_texture_cleanup(X3D_Texture* tex) {
    free(tex->texel.large);
}


