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
} X3D_SlopeVar;

typedef struct X3D_ScanlineValue {
    int16 x;
} X3D_ScanlineValue;

typedef struct X3D_Scanline {
    X3D_ScanlineValue left;
    X3D_ScanlineValue right;
} X3D_Scanline;

typedef struct X3D_RasterEdgeValue {
    float x;
} X3D_RasterEdgeValue;

typedef struct X3D_RasterEdge {
    X3D_RasterEdgeValue value;
    X3D_SlopeVar slope;
} X3D_RasterEdge;

static inline void x3d_rasteredge_advance(X3D_RasterEdge* edge) {
    edge->value.x += edge->slope.x;
}

static inline void x3d_rasteredge_initialize(X3D_RasterEdge* edge, X3D_PolygonRasterVertex* top, X3D_PolygonRasterVertex* bottom) {
    edge->slope.x = ((float)bottom->v.x - top->v.x) / (bottom->v.y - top->v.y);
    edge->value.x = top->v.x;
}

static inline void x3d_rasteredge_initialize_from_scanline(X3D_RasterEdge* edge, X3D_Scanline* scan) {
}

static inline void x3d_scanline_add_edgevalue(X3D_Scanline* scan, X3D_RasterEdgeValue* val) {
    scan->left.x = X3D_MIN(scan->left.x, val->x);
    scan->right.x = X3D_MAX(scan->right.x, val->x);
}

static inline void x3d_rasteredgevalue_draw_pix(X3D_RasterEdgeValue* val, int16 x, int16 y, X3D_PolygonRasterAtt* att) {
    x3d_screen_draw_pix(x, y, att->flat.color);
}

void test_flat_poly() {
    X3D_PolygonRasterVertex v[4] = {
        { .v = { 50, 50 } },
        { .v = { 100, 50 } },
        { .v = { 100, 100 } },
    };
 
    X3D_PolygonRasterAtt at = {
        .flat = {
            .color = x3d_rgb_to_color(0, 0, 255)
        }
    };
    
    x3d_polygon2d_render_flat(v, 3, &at);
}


#define RASTERIZE_NAME x3d_polygon2d_render_flat

#include "render_polygon_generic.c"
