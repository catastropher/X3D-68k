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
#include "render/X3D_surface.h"

static inline void x3d_rasteredgevalue_draw_pix(X3D_RasterEdgeValue* val, int16 x, int16 y, const X3D_PolygonRasterAtt* att) {
    int index = x3d_texture_pixel_index(&att->screen, x, y);
    int16* zbuf = att->zbuf + index;
    X3D_ColorIndex* pix = att->screen.texels + index;
    
    
    if(val->z >> 16 == 0)
        return;
    
    int zz = (1 << 19) / (val->z >> 16);
    
    //printf("ZZ: %d\n", zz);
    
    int uu = val->u >> 16;
    int vv = val->v >> 16;

    //int u = (x3d_surface_w(att->surface.surface) * uu * zz) >> 22;
    //int v = (x3d_surface_h(att->surface.surface) * vv * zz) >> 22;
    
    int u = (((x3d_surface_w(att->surface.surface) * uu) >> 2) * zz) >> 18;
    int v = (((x3d_surface_w(att->surface.surface) * vv) >> 2) * zz) >> 18;
    
    // Should never happen, this is a bug...
    if(u < 0 || v < 0)
        return;
    
    //printf("UV: %d %d\n", uu, vv);
    
    //int u = ((((val->u >> 16) * zz) >> 16) * x3d_surface_w(att->surface.surface)) >> 14;
    //int v = ((((val->v >> 16) * zz) >> 16) * x3d_surface_h(att->surface.surface)) >> 14;
    
    if(zz < *zbuf) {
        X3D_Texture* tex = att->surface.tex;
        
        u = u % tex->w;
        v = v % tex->h;
        
        *pix =  tex->texels[v * tex->w + u];
        *zbuf = zz;
    }
    
    
//     int16 zz = val->z >> 16;
//     
//     if(zz < *zbuf) {
//         X3D_Texture* tex = att->surface.tex;
//         
//         int32 u = (val->u >> 16) % tex->w;
//         int32 v = (val->v >> 16) % tex->h;
//         
//         *pix = tex->texels[v * tex->w + u];
//         *zbuf = zz;
//     }
}

#define RASTERIZE_NAME2D x3d_polygon2d_render_texture_surface
#define RASTERIZE_NAME3D x3d_polygon3d_render_texture_surface

#define PERSPECTIVE_CORRECT
#define NO_CLIP

#include "render_polygon_generic.h"
