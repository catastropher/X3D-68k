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

//#define RASTERIZE_NAME2D x3d_polygon2d_render_texture_surface
//#define RASTERIZE_NAME3D x3d_polygon3d_render_texture_surface

#include "render_polygon_texture_generic.h"

static inline void x3d_rasteredgevalue_draw_pix(X3D_RasterEdgeValue* val, int16 x, int16 y, const X3D_PolygonRasterAtt* att) {
    int index = x3d_texture_pixel_index(&att->screen, x, y);
    X3D_ColorIndex* pix = att->screen.texels + index;

    X3D_Texture* tex = att->texture.texture;
    
    int32 u = (val->u >> 16) % tex->w;
    int32 v = (val->v >> 16) % tex->h;
    
    *pix = tex->texels[v * tex->w + u];
}


#define RASTERIZE_NAME2D x3d_polygon2d_render_texture_repeated
#define RASTERIZE_NAME3D x3d_polygon3d_render_texture_repeated

#include "render_polygon_generic.h"
