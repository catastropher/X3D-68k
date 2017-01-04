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
#include "X3D_polygon.h"

typedef struct X3D_Surface {
    X3D_Texture surface;
} X3D_Surface;

static inline int x3d_surface_w(const X3D_Surface* surface) {
    return surface->surface.w;
}

static inline int x3d_surface_h(const X3D_Surface* surface) {
    return surface->surface.h;
}

static inline X3D_Texture* x3d_surface_texture(X3D_Surface* surface) {
    return &surface->surface;
}

void x3d_surface_init(X3D_Surface* surface, X3D_Polygon3D* poly);
void x3d_surface_cleanup(X3D_Surface* surface);
void x3d_surface_apply_primary_texture(X3D_Surface* surface, X3D_Texture* tex, X3D_TextureOrientation* orientation);

