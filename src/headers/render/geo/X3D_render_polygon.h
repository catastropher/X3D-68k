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

#pragma once

#include "X3D_common.h"
#include "X3D_screen.h"

typedef struct X3D_PolygonRasterVertex {
    X3D_Vex2D v;
    fp0x16 intensity;
    int16 uu, vv;
    int16 zz;
} X3D_PolygonRasterVertex;

typedef struct X3D_PolygonRasterAtt {
    union {
        struct {
            X3D_Color color;
        } flat;
    };
} X3D_PolygonRasterAtt;

static inline void x3d_polygonrastervertex_clamp(X3D_PolygonRasterVertex* v, int16 screen_w, int16 screen_h) {
    v->v.x = X3D_MAX(0, v->v.x);
    v->v.x = X3D_MIN(screen_w, v->v.x);
    
    v->v.y = X3D_MAX(0, v->v.y);
    v->v.y = X3D_MIN(screen_h, v->v.y);
}

void x3d_polygon2d_render_flat(X3D_PolygonRasterVertex v[], uint16 total_v, X3D_PolygonRasterAtt* att);

