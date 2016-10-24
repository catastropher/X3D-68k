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
#include "render/geo/X3D_render_polygon.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"

typedef struct X3D_SlopeVar {
    fp16x16 x;
    fp16x16 z;
    fp16x16 u, v;
    fp16x16 lu, lv;
} X3D_SlopeVar;

typedef struct X3D_ScanlineValue {
    int16 x;
    fp16x16 z;
    fp16x16 u, v;
    fp16x16 lu, lv;
} X3D_ScanlineValue;

typedef struct X3D_Scanline {
    X3D_ScanlineValue left;
    X3D_ScanlineValue right;
} X3D_Scanline;

typedef struct X3D_RasterEdgeValue {
    fp16x16 x;
    fp16x16 z;
    fp16x16 u, v;
    fp16x16 lu, lv;
} X3D_RasterEdgeValue;

typedef struct X3D_RasterEdge {
    X3D_RasterEdgeValue value;
    X3D_SlopeVar slope;
} X3D_RasterEdge;

static inline void x3d_rasteredge_advance(X3D_RasterEdge* edge) {
    edge->value.x += edge->slope.x;
    edge->value.z += edge->slope.z;
    edge->value.u += edge->slope.u;
    edge->value.v += edge->slope.v;
    edge->value.lu += edge->slope.lu;
    edge->value.lv += edge->slope.lv;
}

#include "render/X3D_util.h"

static inline fp16x16 init_slope_var(int16 diff, int16 dx) {
    return ((int32)diff << 16) / dx;
}

static inline void x3d_rasteredge_initialize(X3D_RasterEdge* edge, X3D_PolygonRasterVertex2D* top, X3D_PolygonRasterVertex2D* bottom) {
    int16 dy = X3D_MAX(bottom->v.y - top->v.y, 1);
    
    //top->intensity = x3d_scale_by_depth(0x7FFF, top->zz, 10, 2000);//(1.0 / (top->zz) ;
    //bottom->intensity = x3d_scale_by_depth(0x7FFF, bottom->zz, 10, 2000);
    
    edge->slope.x = init_slope_var(bottom->v.x - top->v.x, dy);
    edge->slope.z = init_slope_var(bottom->zz - top->zz, dy);
    
    edge->slope.u = init_slope_var(bottom->uu - top->uu, dy);
    edge->slope.v = init_slope_var(bottom->vv - top->vv, dy);
    
    edge->slope.lu = init_slope_var(bottom->lu - top->lu, dy);
    edge->slope.lv = init_slope_var(bottom->lv - top->lv, dy);
    
    edge->value.x = ((int32)top->v.x << 16) + 0x8000;
    edge->value.z = (int32)top->zz << 16;
    edge->value.u = ((int32)top->uu << 16);// + 0x8000;
    edge->value.v = ((int32)top->vv << 16);
    edge->value.lu = ((int32)top->lu << 16) + 0x8000;
    edge->value.lv = ((int32)top->lv << 16) + 0x8000;
}

static inline void x3d_rasteredge_initialize_from_scanline(X3D_RasterEdge* edge, X3D_Scanline* scan) {
    int16 dx = X3D_MAX(scan->right.x - scan->left.x, 1);
    
    edge->slope.z = (scan->right.z - scan->left.z) / dx;
    
    edge->slope.u = (scan->right.u - scan->left.u) / dx;
    edge->slope.v = (scan->right.v - scan->left.v) / dx;
    
    edge->slope.lu = (scan->right.lu - scan->left.lu) / dx;
    edge->slope.lv = (scan->right.lv - scan->left.lv) / dx;
    
    edge->value.z = scan->left.z;
    edge->value.u = scan->left.u;
    edge->value.v = scan->left.v;
    edge->value.lu = scan->left.lu;
    edge->value.lv = scan->left.lv;
}

static inline void x3d_scanline_add_edgevalue(X3D_Scanline* scan, X3D_RasterEdgeValue* val) {
    int16 xx = val->x >> 16;
    
    if(xx < scan->left.x) {
        scan->left.x = xx;
        scan->left.z = val->z;
        scan->left.u = val->u;
        scan->left.v = val->v;
        scan->left.lu = val->lu;
        scan->left.lv = val->lv;
    }
    
    if(xx > scan->right.x) {
        scan->right.x = xx;
        scan->right.z = val->z;
        scan->right.u = val->u;
        scan->right.v = val->v;
        scan->right.lu = val->lu;
        scan->right.lv = val->lv;
    }    
}

void test(uint8*);

static inline void x3d_rasteredgevalue_draw_pix(X3D_RasterEdgeValue* val, int16 x, int16 y, const X3D_PolygonRasterAtt* att) {
    //float t = (uint32)x3d_lightmap_get_value(att->light_map.map, val->lu >> 16, val->lv >> 16) / 255.0 + .2;
    
    //test(((uint8 *)att->screen));
    
#if 0
    uint8 r, g, b;
    x3d_color_to_rgb(x3d_texture_get_texel(att->light_map.tex, val->u, val->v), &r, &g, &b);
    
    r = (r * t + .5);
    g = (g * t + .5);
    b = (b * t + .5);
#endif
    
    //X3D_ScreenManager* screenman = x3d_screenmanager_get();
#ifdef __nspire__
    int16* zbuf = att->zbuf + (int32)y * 320 + x;
#else
    int16* zbuf = att->zbuf + (int32)y * 640 + x;
#endif
    
    int16 zz = val->z >> 16;
    
    if(zz < *zbuf) {
        X3D_Texture* tex = att->light_map.tex;
        
        int32 u = (val->u >> 16) % tex->w;
        int32 v = (val->v >> 16) % tex->h;
                
        int32 index = v * tex->w + u;

        

        
#ifdef __nspire__
            ((uint8 *)att->screen)[y * 320 + x] = x3d_colormap_get_index(tex->texel[index],
                x3d_lightmap_get_value(att->light_map.map, val->lu >> 16, val->lv >> 16) / 16 + 2);
#else
            x3d_screen_draw_pix(x, y, x3d_colorindex_to_color(x3d_colormap_get_index(tex->texels[index],
                x3d_lightmap_get_value(att->light_map.map, val->lu >> 16, val->lv >> 16) / 16 + 2)));
#endif
            *zbuf = zz;
        //}
    }
}



#define RASTERIZE_NAME2D x3d_polygon2d_render_texture_lightmap
#define RASTERIZE_NAME3D x3d_polygon3d_render_texture_lightmap


#include "render_polygon_generic.c"
