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

#include <string.h>

#include "memory/X_alloc.h"
#include "geo/X_Vec2.h"

////////////////////////////////////////////////////////////////////////////////
/// An 8-bit color palette index.
////////////////////////////////////////////////////////////////////////////////
typedef unsigned char X_Color;

////////////////////////////////////////////////////////////////////////////////
/// A texture, which is basically a 2D array of texels ("texture elements")
typedef struct X_Texture
{
    int w;              ///< Width of the texture
    int h;              ///< Height of the texture
    X_Color* texels;    ///< Texels
} X_Texture;

_Bool x_texture_save_to_xtex_file(const X_Texture* tex, const char* fileName);
_Bool x_texture_load_from_xtex_file(X_Texture* tex, const char* fileName);

void x_texture_clamp_vec2(const X_Texture* tex, X_Vec2* v);

////////////////////////////////////////////////////////////////////////////////
/// Returns the width of a texture.
////////////////////////////////////////////////////////////////////////////////
static inline int x_texture_w(const X_Texture* tex)
{
    return tex->w;
}

////////////////////////////////////////////////////////////////////////////////
/// Returns the height of a texture.
////////////////////////////////////////////////////////////////////////////////
static inline int x_texture_h(const X_Texture* tex)
{
    return tex->h;
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the total number of texels in a texture.
////////////////////////////////////////////////////////////////////////////////
static inline int x_texture_total_texels(const X_Texture* tex)
{
    return tex->w * tex->h;
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the size of the texels array (in bytes) needed to hold the
///     dimensions for tex.
////////////////////////////////////////////////////////////////////////////////
static inline size_t x_texture_texels_size(const X_Texture* tex)
{
    return x_texture_total_texels(tex) * sizeof(X_Color);
}

////////////////////////////////////////////////////////////////////////////////
/// Initializes a texture given its size.
///
/// @param tex  - texture
/// @param w    - width of the texture
/// @param h    - height of the texture
///
/// @note This allocates memory (for the texels array). Make sure to call
///     @ref x_texture_cleanup() to release the memory!
/// @note This function does not initialize the texels.
////////////////////////////////////////////////////////////////////////////////
static inline void x_texture_init(X_Texture* tex, int w, int h)
{
    tex->w = w;
    tex->h = h;
    tex->texels = x_malloc(x_texture_texels_size(tex));
}

////////////////////////////////////////////////////////////////////////////////
/// Cleans up a texture and releases its memory.
////////////////////////////////////////////////////////////////////////////////
static inline void x_texture_cleanup(X_Texture* tex)
{
    x_free(tex->texels);
    tex->texels = NULL;
    tex->w = 0;
    tex->h = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the index of a texel given its coordinates (u and v).
////////////////////////////////////////////////////////////////////////////////
static inline int x_texture_texel_index(const X_Texture* tex, int u, int v)
{
    return v * tex->w + u;
}

////////////////////////////////////////////////////////////////////////////////
/// Sets the color of a texel in a texture.
////////////////////////////////////////////////////////////////////////////////
static inline void x_texture_set_texel(X_Texture* tex, int u, int v, X_Color color)
{
    tex->texels[x_texture_texel_index(tex, u, v)] = color;
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the color of a texel in a texture.
////////////////////////////////////////////////////////////////////////////////
static inline X_Color x_texture_get_texel(const X_Texture* tex, int u, int v)
{
    return tex->texels[x_texture_texel_index(tex, u, v)];
}

