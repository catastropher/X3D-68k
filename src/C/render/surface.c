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

#include "render/X3D_surface.h"
#include "render/geo/X3D_render_polygon.h"
#include "X3D_trig.h"

typedef struct X3D_SurfaceOrientation {
    X3D_Vex2D s;
    X3D_Vex2D t;
    X3D_Vex2D offset;
    int tex_w;
    int tex_h;
} X3D_SurfaceOrientation;

X3D_Vex2D x3d_surfaceorientation_project_point(X3D_SurfaceOrientation* orientation, X3D_Vex2D point) {
    X3D_Vex2D v = x3d_vex2d_make(
        x3d_vex2d_dot_shifted(&point, &orientation->s) + orientation->offset.x,
        x3d_vex2d_dot_shifted(&point, &orientation->t) + orientation->offset.y
    );
    
    return v;
}

void x3d_surface_adjust_texture_coordinates(X3D_PolygonRasterVertex2D v[4], X3D_SurfaceOrientation* orientation) {
    int min_x = v[0].uu;
    int min_y = v[0].vv;
    
    for(int i = 1; i < 4; ++i) {
        min_x = X3D_MIN(min_x, v[i].uu);
        min_y = X3D_MIN(min_y, v[i].vv);
    }
    
    int add_x = 0;
    int add_y = 0;
    
    while(min_x + add_x < 0)
        add_x += orientation->tex_w;
    
    while(min_y + add_y < 0)
        add_y += orientation->tex_h;
    
    for(int i = 0; i < 4; ++i) {
        v[i].uu += add_x;
        v[i].vv += add_y;
    }
}

void x3d_surfaceorientation_from_textureorientation(X3D_SurfaceOrientation* surface_orientation, X3D_TextureOrientation* tex_orientation, X3D_Texture* tex) {
    surface_orientation->s = x3d_vex2d_make(x3d_cos(tex_orientation->angle), x3d_sin(tex_orientation->angle));
    surface_orientation->t = x3d_vex2d_rotate_clockwise_by_90_degrees(surface_orientation->s);
    surface_orientation->offset = tex_orientation->offset;
    surface_orientation->tex_w = tex->w;
    surface_orientation->tex_h = tex->h;
}

void x3d_surfaceorientation_calculate_primary_texture_coordinates(X3D_Surface* surface, X3D_SurfaceOrientation* surface_orientation, X3D_PolygonRasterVertex2D v[4]) {
    int w = x3d_surface_w(surface);
    int h = x3d_surface_h(surface);
    
    X3D_Vex2D center = x3d_vex2d_make(w / 2, h / 2);
    
    X3D_Vex2D bounds[4] = {
        x3d_vex2d_make(0, 0),
        x3d_vex2d_make(w, 0),
        x3d_vex2d_make(w, h),
        x3d_vex2d_make(0, h)
    };
    
    for(int i = 0; i < 4; ++i) {
        X3D_Vex2D texture_coord = x3d_surfaceorientation_project_point(surface_orientation, x3d_vex2d_sub(bounds + i, &center));
        x3d_polygonrastervertex2d_set_texture_coords(v + i, texture_coord);
        
        x3d_polygonrastervertex2d_set_vertex(v + i, bounds[i]);
    }
    
    x3d_surface_adjust_texture_coordinates(v, surface_orientation);
}

void x3d_surface_apply_primary_texture(X3D_Surface* surface, X3D_Texture* tex, X3D_TextureOrientation* orientation) {
    X3D_SurfaceOrientation surface_orientation;
    x3d_surfaceorientation_from_textureorientation(&surface_orientation, orientation, tex);
    
    X3D_PolygonRasterVertex2D v[4];
    x3d_surfaceorientation_calculate_primary_texture_coordinates(surface, &surface_orientation, v);
    
    X3D_PolygonRasterAtt att;
    x3d_polygonrasteratt_set_screen(&att, x3d_surface_texture(surface), NULL);
    
    att.texture.texture = tex;
    
    x3d_polygon2d_render_texture_repeated(v, 4, &att);
}

void x3d_surface_init(X3D_Surface* surface, X3D_Polygon3D* poly) {
    x3d_texture_init(x3d_surface_texture(surface), 200, 200, 0);
}

void x3d_surface_cleanup(X3D_Surface* surface) {
    x3d_texture_cleanup(x3d_surface_texture(surface));
}

