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
#include "render/X3D_texture.h"
#include "render/X3D_lightmap.h"

typedef struct X3D_PolygonRasterVertex2D {
    X3D_Vex2D v;
    fp0x16 intensity;
    int16 uu, vv;
    int16 zz;
    int16 lu, lv;
} X3D_PolygonRasterVertex2D;

typedef struct X3D_PolygonRasterVertex3D {
    X3D_Vex3D v;
    fp0x16 intensity;
    int16 uu, vv;
    int16 zz;
    int16 lu, lv;
} X3D_PolygonRasterVertex3D;

typedef struct X3D_RasterPolygon2D {
    uint16 total_v;
    X3D_PolygonRasterVertex2D* v;
} X3D_RasterPolygon2D;

typedef struct X3D_RasterPolygon3D {
    uint16 total_v;
    X3D_PolygonRasterVertex3D* v;
} X3D_RasterPolygon3D;

typedef struct X3D_PolygonRasterAtt {
    union {
        struct {
            X3D_Color color;
        } flat;
        
        struct {
            X3D_Texture* texture;
        } texture;
        
        struct {
            uint32 id;
        } id_buffer;
        
        struct {
            X3D_LightMap* map;
            X3D_Texture* tex;
        } light_map;
        
        struct {
            int16 z;
        } zfill;
    };
    
    int16* zbuf;
    void* screen;
    X3D_Frustum* frustum;
} X3D_PolygonRasterAtt;

static inline void x3d_polygonrastervertex_clamp(X3D_PolygonRasterVertex2D* v, int16 screen_w, int16 screen_h) {
    v->v.x = X3D_MAX(0, v->v.x);
    v->v.x = X3D_MIN(screen_w - 1, v->v.x);
    
    v->v.y = X3D_MAX(0, v->v.y);
    v->v.y = X3D_MIN(screen_h - 1, v->v.y);
}

static inline void x3d_polygonrastervertex3d_copy_attributes(X3D_PolygonRasterVertex3D* src, X3D_PolygonRasterVertex2D* dest) {
    dest->uu = src->uu;
    dest->vv = src->vv;
    dest->zz = src->v.z;
    dest->intensity = src->intensity;
    dest->lu = src->lu;
    dest->lv = src->lv;
}

struct X3D_CameraObject;

void x3d_polygon2d_render_flat(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att);
void x3d_polygon3d_render_flat(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, struct X3D_CameraObject* cam);

void x3d_polygon2d_render_gouraud(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att);
void x3d_polygon3d_render_gouraud(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, struct X3D_CameraObject* cam);

void x3d_polygon2d_render_texture(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att);
void x3d_polygon3d_render_texture(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, struct X3D_CameraObject* cam);

void x3d_polygon2d_render_id_buffer(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att);
void x3d_polygon3d_render_id_buffer(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, struct X3D_CameraObject* cam);

void x3d_polygon2d_render_lightmap(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att);
void x3d_polygon3d_render_lightmap(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, struct X3D_CameraObject* cam);

void x3d_polygon2d_render_texture_lightmap(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att);
void x3d_polygon3d_render_texture_lightmap(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, struct X3D_CameraObject* cam);

void x3d_polygon2d_render_zfill(X3D_PolygonRasterVertex2D v[], uint16 total_v, X3D_PolygonRasterAtt* att);
void x3d_polygon3d_render_zfill(X3D_RasterPolygon3D* poly, X3D_PolygonRasterAtt* att, struct X3D_CameraObject* cam);

