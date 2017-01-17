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
#include "render/geo/X3D_render_polygon.h"
#include "render/geo/X3D_clip_polygon.h"

#include <SDL/SDL.h>

static inline void add_edge(X3D_RasterEdge* edge, X3D_Scanline* start, X3D_Scanline* end) {
    while(start < end) {
        x3d_scanline_add_edgevalue(start, &edge->value);
        x3d_rasteredge_advance(edge);
        ++start;
    }
}

static inline void render_scanline(X3D_Scanline* scan, int16 y, X3D_PolygonRasterAtt* att) {
    X3D_RasterEdge edge;
    x3d_rasteredge_initialize_from_scanline(&edge, scan);
    
    for(int i = scan->left.x; i < scan->right.x; ++i) {
        x3d_rasteredgevalue_draw_pix(&edge.value, i, y, att);
        x3d_rasteredge_advance(&edge);
    }
}

void RASTERIZE_NAME2D(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att) {    
    int min_y = 0x7FFF;
    int max_y = -0x7FFF;
    
    int screen_w = x3d_texture_w(&att->screen);
    int screen_h = x3d_texture_h(&att->screen);
    
    for(int i = 0; i < total_v; ++i) {
        uint16 next = (i + 1) % total_v;
        X3D_PolygonRasterVertex2D* top    = v + i;
        X3D_PolygonRasterVertex2D* bottom = v + next;
        
        x3d_polygonrastervertex_clamp(top, screen_w, screen_h);
        x3d_polygonrastervertex_clamp(bottom, screen_w, screen_h);
        
        min_y = X3D_MIN(min_y, top->v.y);
        max_y = X3D_MAX(max_y, bottom->v.y);
    }
    
    int h = max_y - min_y + 1;
    X3D_Scanline scans[h];
    
    for(int i = 0; i < h; ++i) {
        scans[i].left.x = 0x7FFF;
        scans[i].right.x = -0x7FFF;
    }
    
    for(int i = 0; i < total_v; ++i) {
        uint16 next = (i + 1) % total_v;
        X3D_RasterEdge edge;
        X3D_PolygonRasterVertex2D* top    = v + i;
        X3D_PolygonRasterVertex2D* bottom = v + next;
        
        if(v[i].v.y > v[next].v.y)
            X3D_SWAP(top, bottom);
        
        x3d_rasteredge_initialize(&edge, top, bottom);
        add_edge(&edge, scans + top->v.y - min_y, scans + bottom->v.y - min_y);
    }
    
    for(int i = min_y; i < max_y; ++i) {
        render_scanline(scans + i - min_y, i, att);
    }
}

void RASTERIZE_NAME3D(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, X3D_CameraObject* cam) {
    X3D_PolygonRasterVertex3D clipped_v[20];
    X3D_RasterPolygon3D clipped_poly = { .v = clipped_v };
    
#ifndef NO_CLIP
    x3d_rasterpolygon3d_clip_to_frustum(poly, att->frustum, &clipped_poly);
#else
    clipped_poly = *poly;
#endif
    
    X3D_PolygonRasterVertex2D projected_v[20];
    
    uint16 i;
    for(i = 0; i < clipped_poly.total_v; ++i) {
        X3D_Vex3D rotated;
        x3d_camera_transform_points(cam, &clipped_poly.v[i].v, 1, &rotated, &projected_v[i].v);
        x3d_polygonrastervertex3d_copy_attributes(clipped_poly.v + i, projected_v + i);
        
#ifndef PERSPECTIVE_CORRECT
        projected_v[i].zz = rotated.z;
#else
        const int ONE_FP = (1 << 20);
        projected_v[i].zz = ONE_FP / rotated.z;
        
        projected_v[i].uu = (projected_v[i].uu * projected_v[i].zz) >> 14;
        projected_v[i].vv = (projected_v[i].vv * projected_v[i].zz) >> 14;
        
        //printf("UV: %d %d\n", projected_v[i].uu, projected_v[i].vv);
#endif
    }
    
    RASTERIZE_NAME2D(projected_v, clipped_poly.total_v, att);
}

