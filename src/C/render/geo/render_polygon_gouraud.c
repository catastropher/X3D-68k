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

typedef struct X3D_SlopeVar {
    float x;
    float intensity;
} X3D_SlopeVar;

typedef struct X3D_ScanlineValue {
    int16 x;
    fp0x16 intensity;
} X3D_ScanlineValue;

typedef struct X3D_Scanline {
    X3D_ScanlineValue left;
    X3D_ScanlineValue right;
} X3D_Scanline;

typedef struct X3D_RasterEdgeValue {
    float x;
    float intensity;
} X3D_RasterEdgeValue;

typedef struct X3D_RasterEdge {
    X3D_RasterEdgeValue value;
    X3D_SlopeVar slope;
} X3D_RasterEdge;

static inline void x3d_rasteredge_advance(X3D_RasterEdge* edge) {
    edge->value.x += edge->slope.x;
    edge->value.intensity += edge->slope.intensity;
}

static inline void x3d_rasteredge_initialize(X3D_RasterEdge* edge, X3D_PolygonRasterVertex2D* top, X3D_PolygonRasterVertex2D* bottom) {
    int16 dy = X3D_MAX(bottom->v.y - top->v.y, 1);
    
    edge->slope.x = ((float)bottom->v.x - top->v.x) / dy;
    edge->slope.intensity = ((float)bottom->intensity - top->intensity) / dy;
    
    edge->value.x = top->v.x;
    edge->value.intensity = top->intensity;
}

static inline void x3d_rasteredge_initialize_from_scanline(X3D_RasterEdge* edge, X3D_Scanline* scan) {
    int16 dx = X3D_MAX(scan->right.x - scan->left.x, 1);
    
    edge->slope.intensity = ((float)scan->right.intensity - scan->left.intensity) / dx;
    edge->value.intensity = scan->left.intensity;
}

static inline void x3d_scanline_add_edgevalue(X3D_Scanline* scan, X3D_RasterEdgeValue* val) {
    if(val->x <scan->left.x) {
        scan->left.x = val->x;
        scan->left.intensity = val->intensity;
    }
    
    if(val->x > scan->right.x) {
        scan->right.x = val->x;
        scan->right.intensity = val->intensity;
    }    
}

static inline void x3d_rasteredgevalue_draw_pix(X3D_RasterEdgeValue* val, int16 x, int16 y, X3D_PolygonRasterAtt* att) {
    uint8 r, g, b;
    x3d_color_to_rgb(att->flat.color, &r, &g, &b);
    
    float t = val->intensity / 32768.0;
    
    r *= t;
    g *= t;
    b *= t;
    
    x3d_screen_draw_pix(x, y, x3d_rgb_to_color(r, g, b));
}



#define RASTERIZE_NAME2D x3d_polygon2d_render_gouraud
#define RASTERIZE_NAME3D x3d_polygon3d_render_gouraud


#include "render_polygon_generic.c"
