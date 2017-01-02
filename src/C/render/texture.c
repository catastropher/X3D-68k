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

_Bool x3d_texture_load_from_bmp_file(X3D_Texture* tex, const char* file) {
    return x3d_platform_screen_load_texture(tex, file);
}

void x3d_texture_blit(X3D_Texture* tex, uint16 x, uint16 y) {    
    for(int i = 0; i < tex->h; ++i) {
        for(int d = 0; d < tex->w; ++d) {
            x3d_screen_draw_pix(d + x, i + y, x3d_texture_get_texel(tex, d, i));
        }
    }
}

void x3d_texture_init(X3D_Texture* tex, uint16 w, uint16 h, uint16 flags) {
    tex->w = w;
    tex->h = h;
    tex->texels = malloc(sizeof(X3D_ColorIndex) * w * h);
    tex->flags = flags;
}

void x3d_texture_cleanup(X3D_Texture* tex) {
    free(tex->texels);
    x3d_texture_init_empty(tex);
}

void x3d_texture_fill(X3D_Texture* tex, X3D_ColorIndex color) {
    memset(tex->texels, color, (uint32)tex->w * tex->h);
}

void x3d_texture_copy_texels(X3D_Texture* dest, const X3D_Texture* src) {
    x3d_assert(dest->w == src->w && dest->h == src->h);
    memcpy(dest->texels, src->texels, sizeof(X3D_ColorIndex) * x3d_texture_total_texels(src));
}

