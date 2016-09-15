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
#include "X3D_screen.h"
#include "X3D_polygon.h"
#include "X3D_enginestate.h"

typedef struct X3D_RenderVertex3D {
    int16 uu, vu;
    int16 z;
    int16 i;
    X3D_Vex3D v;
} X3D_RenderVertex3D;

typedef struct X3D_RenderSpanValue {
    int16 u, v;
    int16 x;
    int16 z;
    fp0x16 i;
} X3D_RenderSpanValue;

typedef struct X3D_RenderSpan {
    X3D_RenderSpanValue left;
    X3D_RenderSpanValue right;
} X3D_RenderSpan;

float x3d_calculate_slope(int16 start, int16 end, int16 dx) {
    if(dx == 0) return 0;
    
    return ((float)end - start) / dx;
}

X3D_Color x3d_color_scale(X3D_Color c, fp0x16 scale) {
    uint8 r, g, b;
    x3d_color_to_rgb(c, &r, &g, &b);
    
    return x3d_rgb_to_color(
        ((uint16)r * scale) >> 15,
        ((uint16)g * scale) >> 15,
        ((uint16)b * scale) >> 15
    );
}

void x3d_renderspan_render(X3D_RenderSpan* span, int16 y, X3D_Color c) {
    int16 dx = span->right.x - span->left.x;
    
    float i_slope = x3d_calculate_slope(span->left.i, span->right.i, dx);
    float u_slope = x3d_calculate_slope(span->left.u, span->right.u, dx);
    float v_slope = x3d_calculate_slope(span->left.v, span->right.v, dx);
    float z_slope = x3d_calculate_slope(span->left.z, span->right.z, dx);
    
    float z = span->left.z;
    float u = span->left.u;
    float v = span->left.v;
    float i = span->left.i;
    
    int16 x;
    for(x = span->left.x; x <= span->right.x; ++x) {
        x3d_screen_draw_pix(x, y, i);
        
        i += i_slope;
        u += u_slope;
        v += v_slope;
        z += z_slope;
    }
}

void x3d_polygon3d_render(X3D_Polygon3D* poly) {
    X3D_Vex2D projected_v[poly->total_v];
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i)
        x3d_vex3d_int16_project(projected_v + i, poly->v + i);
    
    // Oh god, this is terrible...
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_RenderSpan spans[screenman->h];
    
    for(i = 0; i < screenman->h; ++i) {
        spans[i].left.x = 0x7FFF;
        spans[i].right.x = -0x7FFF;
    }
    
    for(i = 0; i < poly->total_v; ++i) {
        uint16 next = (i + 1) % poly->total_v;
        
        float i_slope = ()
    }
}


