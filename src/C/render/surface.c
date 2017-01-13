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
#include "geo/X3D_boundrect.h"

static void adjust_texture_coordinates_so_all_positive(X3D_PolygonRasterVertex2D v[4], int tex_w, int tex_h) {
    int min_x = v[0].uu;
    int min_y = v[0].vv;
    
    for(int i = 1; i < 4; ++i) {
        min_x = X3D_MIN(min_x, v[i].uu);
        min_y = X3D_MIN(min_y, v[i].vv);
    }
    
    int add_x = 0;
    int add_y = 0;
    
    while(min_x + add_x < 0)
        add_x += tex_w;
    
    while(min_y + add_y < 0)
        add_y += tex_h;
    
    for(int i = 0; i < 4; ++i) {
        v[i].uu += add_x;
        v[i].vv += add_y;
    }
}

static void draw_texture_on_surface(X3D_Surface* surface, X3D_Texture* texture, X3D_PolygonRasterVertex2D v[4], const X3D_BoundRect* update_rect) {
    X3D_PolygonRasterAtt att;
    x3d_polygonrasteratt_init(&att);
    x3d_polygonrasteratt_set_screen(&att, x3d_surface_texture(surface), NULL);
    
    att.texture.texture = texture;
    
    x3d_polygon2d_render_texture_repeated(v, 4, &att);
}

static void draw_decal_on_surface(X3D_Surface* surface, X3D_SurfaceTexture* decal, const X3D_BoundRect* update_rect) {
    
}

int x3d_orientation2d_calculate_transformed_coordinate(X3D_Vex2D point, X3D_Vex2D axis, int scale, int offset) {
    return (((int)x3d_vex2d_dot_shifted(&point, &axis) << 8) / scale) + offset;
}

static X3D_Vex2D x3d_orientation2d_transform_point(const X3D_Orientation2D* orientation, X3D_Vex2D point) {
    return x3d_vex2d_make(
        x3d_orientation2d_calculate_transformed_coordinate(point, orientation->s, orientation->scale, orientation->offset.x),
        x3d_orientation2d_calculate_transformed_coordinate(point, orientation->t, orientation->scale, orientation->offset.y)
    );
}

static void x3d_orientation2d_make_from_surfacetexture(X3D_Orientation2D* orientation, const X3D_SurfaceTexture* tex) {
    orientation->offset = tex->offset;
    orientation->scale = tex->scale;
    
    orientation->s = x3d_vex2d_make(x3d_cos(tex->angle), x3d_sin(tex->angle));
    orientation->t = x3d_vex2d_rotate_clockwise_by_90_degrees(orientation->s);
}

static void populate_rastervertices_with_extent_of_surface(X3D_PolygonRasterVertex2D v[4], X3D_Surface* surface) {
    int w = x3d_surface_w(surface);
    int h = x3d_surface_h(surface);
    
    v[0].v = x3d_vex2d_make(0, 0);
    v[1].v = x3d_vex2d_make(w, 0);
    v[2].v = x3d_vex2d_make(w, h);
    v[3].v = x3d_vex2d_make(0, h);
}

static void calculate_primary_texture_coordinates_from_vertices(X3D_PolygonRasterVertex2D v[4], const X3D_Surface* surface, const X3D_Orientation2D* orientation) {
    X3D_Vex2D center = x3d_vex2d_make(x3d_surface_w(surface) / 2, x3d_surface_h(surface) / 2);
    
    for(int i = 0; i < 4; ++i) {
        X3D_Vex2D point_relative_to_center = x3d_vex2d_sub(&v[i].v, &center);
        X3D_Vex2D transformed_point = x3d_orientation2d_transform_point(orientation, point_relative_to_center);
        
        x3d_polygonrastervertex2d_set_texture_coords(v + i, transformed_point);
    }
}

static void draw_primary_texture_on_surface(X3D_Surface* surface, X3D_SurfaceTexture* primary_texture, const X3D_BoundRect* update_rect) {
    X3D_Orientation2D orientation;
    x3d_orientation2d_make_from_surfacetexture(&orientation, primary_texture);
    
    X3D_PolygonRasterVertex2D v[4];
    populate_rastervertices_with_extent_of_surface(v, surface);
    calculate_primary_texture_coordinates_from_vertices(v, surface, &orientation);
    adjust_texture_coordinates_so_all_positive(v, x3d_surfacetexture_w(primary_texture), x3d_surfacetexture_h(primary_texture));
    draw_texture_on_surface(surface, primary_texture->tex, v, update_rect);
}

static void fill_surface_with_color(X3D_Surface* surface, X3D_ColorIndex color, const X3D_BoundRect* update_rect) {
    
}

static void draw_surfacetexture_on_surface(X3D_Surface* surface, X3D_SurfaceTexture* texture, const X3D_BoundRect* update_rect) {
    if(x3d_surfacetexture_is_decal(texture))
        draw_decal_on_surface(surface, texture, update_rect);
    else
        draw_primary_texture_on_surface(surface, texture, update_rect);
}

static void build_surface(X3D_Surface* surface, const X3D_BoundRect* update_rect) {
    for(int i = 0; i < surface->total_textures; ++i) {
        draw_surfacetexture_on_surface(surface, surface->textures + i, update_rect);
    }
}

void x3d_surface_force_entire_rebuild(X3D_Surface* surface) {
    x3d_surface_set_flags(surface, X3D_SURFACE_REBUILD_ENTIRE);
    build_surface(surface, NULL);
}

void x3d_surface_rebuild_region(X3D_Surface* surface, const X3D_BoundRect* region) {
    build_surface(surface, region);
}

void x3d_surface_init(X3D_Surface* surface, X3D_Polygon3D* poly) {
    x3d_texture_init(x3d_surface_texture(surface), 200, 200, 0);
}

void x3d_surface_cleanup(X3D_Surface* surface) {
    x3d_texture_cleanup(x3d_surface_texture(surface));
}

