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
#include "render/X3D_palette.h"

enum {
  X3D_TEXTURE_REPEAT = 1
};

///////////////////////////////////////////////////////////////////////////////
/// A texture.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Texture {
  uint16 w, h;          ///< Width and height of the texture
  uint16 mask;          ///< Mask for repeated textures
  uint16 flags;
  
  X3D_ColorIndex* texels;
  
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
    if(tex->flags & X3D_TEXTURE_REPEAT)
        return (uint32)(v & tex->mask) * tex->w + (u & tex->mask);
    else
        return (uint32)v * tex->w + u;
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
static inline X3D_ColorIndex x3d_texture_get_texel(const X3D_Texture* tex, uint16 u, uint16 v) {  
    return tex->texels[x3d_texture_index(tex, u, v)];
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
static inline void x3d_texture_set_texel(X3D_Texture* tex, uint16 u, uint16 v, X3D_ColorIndex c) {
  tex->texels[x3d_texture_index(tex, u, v)] = c;
}

static inline void x3d_texture_init_empty(X3D_Texture* tex) {
    tex->w = 0;
    tex->h = 0;
    tex->texels = NULL;
}

static inline _Bool x3d_texture_is_empty(const X3D_Texture* tex) {
    return tex->texels == NULL;
}

static inline uint32 x3d_texture_total_texels(const X3D_Texture* tex) {
    return (uint32)tex->w * tex->h;
}

static inline int16 x3d_texture_texel_is_valid(const X3D_Texture* tex, int16 u, int16 v) {
    return u >= 0 && v >= 0 && u < tex->w && v < tex->h;
}

_Bool x3d_texture_load_from_file(X3D_Texture* tex, const char* file);
void x3d_texture_blit(X3D_Texture* tex, uint16 x, uint16 y);
void x3d_texture_to_array(X3D_Texture* texture, FILE* file, const char* name);
void x3d_texture_from_array(X3D_Texture* dest, uint8* data);
void x3d_texture_init(X3D_Texture* tex, uint16 w, uint16 h, uint16 flags);
void x3d_texture_cleanup(X3D_Texture* tex);
void x3d_texture_fill(X3D_Texture* tex, X3D_ColorIndex color);
void x3d_texture_copy_texels(X3D_Texture* dest, const X3D_Texture* src);
void x3d_texture_create_new(X3D_Texture* tex, int16 w, int16 h, X3D_Color color);
void x3d_texture_fill_with_checkerboard(X3D_Texture* tex, int16 width, int16 square_size);

