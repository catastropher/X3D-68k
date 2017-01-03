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
} X3D_SlopeVar;

typedef struct X3D_ScanlineValue {
    int16 x;
} X3D_ScanlineValue;

typedef struct X3D_Scanline {
    X3D_ScanlineValue left;
    X3D_ScanlineValue right;
} X3D_Scanline;

typedef struct X3D_RasterEdgeValue {
    fp16x16 x;
} X3D_RasterEdgeValue;

typedef struct X3D_RasterEdge {
    X3D_RasterEdgeValue value;
    X3D_SlopeVar slope;
} X3D_RasterEdge;

static inline void x3d_rasteredge_advance(X3D_RasterEdge* edge) {
    edge->value.x += edge->slope.x;
}

#include "render/X3D_util.h"

static inline fp16x16 init_slope_var(int16 diff, int16 dx) {
    return ((int32)diff << 16) / dx;
}

static inline void x3d_rasteredge_initialize(X3D_RasterEdge* edge, X3D_PolygonRasterVertex2D* top, X3D_PolygonRasterVertex2D* bottom) {
    int16 dy = X3D_MAX(bottom->v.y - top->v.y, 1);
    
    edge->slope.x = init_slope_var(bottom->v.x - top->v.x, dy);
    edge->value.x = (int32)top->v.x << 16;
}

static inline void x3d_rasteredge_initialize_from_scanline(X3D_RasterEdge* edge, X3D_Scanline* scan) {
}

static inline void x3d_scanline_add_edgevalue(X3D_Scanline* scan, X3D_RasterEdgeValue* val) {
    int16 xx = val->x >> 16;
    
    if(xx < scan->left.x) {
        scan->left.x = xx;
    }
    
    if(xx > scan->right.x) {
        scan->right.x = xx;
    }    
}

static inline void x3d_rasteredgevalue_draw_pix(X3D_RasterEdgeValue* val, int16 x, int16 y, const X3D_PolygonRasterAtt* att) {
#ifdef __nspire__
    int16* zbuf = att->zbuf + (int32)y * 320 + x;
#else
    int16* zbuf = att->zbuf + (int32)y * 640 + x;
#endif
    *zbuf = att->zfill.z;
}



#define RASTERIZE_NAME2D x3d_polygon2d_render_zfill
#define RASTERIZE_NAME3D x3d_polygon3d_render_zfill


#include "render_polygon_generic.h"
