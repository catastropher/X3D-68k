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
#include "X3D_screen.h"

enum {
  X3D_TEXTURE_4BIT = 1
};

///////////////////////////////////////////////////////////////////////////////
/// A texture.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Texture {
  uint16 w, h;          ///< Width and height of the texture
  uint16 mask;          ///< Mask for repeated textures
  uint16 flags;
  
  union {               ///< Texels (texture elements, like pixels for textures)
    uint8* small;
    uint8* large;
  } texel;
  
  X3D_Color* color_tab;
  uint16 total_c;
} X3D_Texture;

///////////////////////////////////////////////////////////////////////////////
/// Calculates the index into the texel array given a texture's u and v
///   coordinate.
///
/// @param tex  - texture
/// @param u    - x coordinate in texture
/// @param v    - y coordinate in texture
///
/// @return The index in the texel array of the desired texel.
///////////////////////////////////////////////////////////////////////////////
static inline uint32 x3d_texture_index(const X3D_Texture* tex, uint16 u, uint16 v) {
  return (uint32)(v & tex->mask) * tex->w + (u & tex->mask);
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the color of a texel in a texture.
///
/// @param tex  - texture
/// @param u    - x coordinate in texture
/// @param v    - y coordinate in texture
///
/// @return The color of the desired texel.
///////////////////////////////////////////////////////////////////////////////
static inline X3D_Color x3d_texture_get_texel(const X3D_Texture* tex, uint16 u, uint16 v) {
  if(tex->flags & X3D_TEXTURE_4BIT) {
    uint8 byte = tex->texel.small[(v * tex->w + u) >> 1];
   
    if((u & 1) == 0)
      byte >>= 4;
    
    return tex->color_tab[byte & 0x0F];
  }
  
  return tex->color_tab[tex->texel.large[x3d_texture_index(tex, u, v)]];
}

static inline X3D_Color x3d_texture_get_texel_128(const X3D_Texture* tex, uint16 u, uint16 v) {
  //return tex->texel[(uint32)(v & tex->mask) * 128 + (u & tex->mask)];
  return 31;
}

///////////////////////////////////////////////////////////////////////////////
/// Sets the color of a texel in a texture.
///
/// @param tex  - texture
/// @param u    - x coordinate in texture
/// @param v    - y coordinate in texture
/// @param c    - color
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
static inline void x3d_texture_set_texel(X3D_Texture* tex, uint16 u, uint16 v, X3D_Color c) {
  //tex->texel[x3d_texture_index(tex, u, v)] = c;
}

_Bool x3d_texture_load_from_file(X3D_Texture* tex, const char* file);
void x3d_texture_blit(X3D_Texture* tex, uint16 x, uint16 y);
void x3d_texture_to_array(X3D_Texture* texture, FILE* file, const char* name);
void x3d_texture_from_array(X3D_Texture* dest, uint8* data);


