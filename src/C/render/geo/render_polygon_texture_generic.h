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
} X3D_SlopeVar;

typedef struct X3D_ScanlineValue {
    int16 x;
    fp16x16 z;
    fp16x16 u, v;
} X3D_ScanlineValue;

typedef struct X3D_Scanline {
    X3D_ScanlineValue left;
    X3D_ScanlineValue right;
} X3D_Scanline;

typedef struct X3D_RasterEdgeValue {
    fp16x16 x;
    fp16x16 z;
    fp16x16 u, v;
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
}

#include "render/X3D_util.h"

static inline fp16x16 init_slope_var(int16 diff, int16 dx) {
    return ((int32)diff << 16) / dx;
}

static inline void x3d_rasteredge_initialize(X3D_RasterEdge* edge, X3D_PolygonRasterVertex2D* top, X3D_PolygonRasterVertex2D* bottom) {
    int16 dy = X3D_MAX(bottom->v.y - top->v.y, 1);
    
    edge->slope.x = init_slope_var(bottom->v.x - top->v.x, dy);
    edge->slope.z = init_slope_var(bottom->zz - top->zz, dy);
    
    edge->slope.u = init_slope_var(bottom->uu - top->uu, dy);
    edge->slope.v = init_slope_var(bottom->vv - top->vv, dy);
    
    edge->value.x = ((int32)top->v.x << 16) + 0x8000;
    edge->value.z = (int32)top->zz << 16;
    edge->value.u = ((int32)top->uu << 16);// + 0x8000;
    edge->value.v = ((int32)top->vv << 16);
}

static inline void x3d_rasteredge_initialize_from_scanline(X3D_RasterEdge* edge, X3D_Scanline* scan) {
    int16 dx = X3D_MAX(scan->right.x - scan->left.x, 1);
    
    edge->slope.z = (scan->right.z - scan->left.z) / dx;
    
    edge->slope.u = (scan->right.u - scan->left.u) / dx;
    edge->slope.v = (scan->right.v - scan->left.v) / dx;
        
    edge->value.z = scan->left.z;
    edge->value.u = scan->left.u;
    edge->value.v = scan->left.v;
}

static inline void x3d_scanline_add_edgevalue(X3D_Scanline* scan, X3D_RasterEdgeValue* val) {
    int16 xx = val->x >> 16;
    
    if(xx < scan->left.x) {
        scan->left.x = xx;
        scan->left.z = val->z;
        scan->left.u = val->u;
        scan->left.v = val->v;
    }
    
    if(xx > scan->right.x) {
        scan->right.x = xx;
        scan->right.z = val->z;
        scan->right.u = val->u;
        scan->right.v = val->v;
    }    
}

