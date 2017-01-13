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

typedef struct X3D_Orientation2D {
    X3D_Vex2D s;
    X3D_Vex2D t;
    X3D_Vex2D offset;
    int scale;          // fixed-point in 8.8 format
} X3D_Orientation2D;

typedef enum X3D_SurfaceTextureFlags {
    X3D_SURFACETEXTURE_IS_DECAL = 1
} X3D_SurfaceTextureFlags;

typedef struct X3D_SurfaceTexture {
    X3D_Vex2D offset;
    X3D_Texture* tex;
    fp8x8 scale;
    angle256 angle;
    uint8 flags;
} X3D_SurfaceTexture;

static inline _Bool x3d_surfacetexture_is_decal(X3D_SurfaceTexture* tex) {
    return tex->flags & X3D_SURFACETEXTURE_IS_DECAL;
}


static inline int x3d_surfacetexture_w(X3D_SurfaceTexture* tex) {
    return tex->tex->w;
}

static inline int x3d_surfacetexture_h(X3D_SurfaceTexture* tex) {
    return tex->tex->h;
}

typedef enum X3D_SurfaceFlags {
    X3D_SURFACE_REBUILD_ENTIRE = 1
} X3D_SurfaceFlags;

typedef struct X3D_Surface {
    X3D_Texture surface;
    X3D_SurfaceTexture* textures;
    int total_textures;
    int flags;
} X3D_Surface;

static inline _Bool x3d_surface_needs_entirely_rebuilt(const X3D_Surface* surface) {
    return surface->flags & X3D_SURFACE_REBUILD_ENTIRE;
}

static inline void x3d_surface_set_flags(X3D_Surface* surface, X3D_SurfaceFlags flags) {
    surface->flags |= flags;
}

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
void x3d_surface_force_entire_rebuild(X3D_Surface* surface);

