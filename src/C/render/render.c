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
#include "X3D_prism.h"
#include "X3D_camera.h"
#include "X3D_enginestate.h"
#include "X3D_clip.h"
#include "X3D_trig.h"
#include "X3D_collide.h"
#include "X3D_wallportal.h"
#include "X3D_portal.h"
#include "X3D_object.h"
#include "X3D_fastsqrt.h"
#include "X3D_polygon.h"
#include "level/X3D_level.h"
#include "render/X3D_font.h"
#include "render/geo/X3D_render_prism.h"
#include "render/geo/X3D_render_linetexture.h"
#include "render/geo/X3D_render_polygon.h"
#include "render/geo/X3D_clip_polygon.h"

#include "geo/X3D_line.h"

#include <stdio.h>


extern int16 render_mode;
extern uint16 geo_render_mode;

void x3d_rendermanager_init(X3D_InitSettings* settings) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  X3D_ScreenManager* screenman = x3d_screenmanager_get();
  
  // Initialize the render stack
  uint32 stack_size = 600000;
  void* render_stack_mem = malloc(stack_size);

  x3d_assert(render_stack_mem);

  x3d_stack_init(&renderman->stack, render_stack_mem, stack_size);
  
  // Reset segment face render callback
  renderman->segment_face_render_callback = NULL;
  

  int16 offx = 0, offy = 0;


  screenman->w = settings->screen_w;
  screenman->h = settings->screen_h;

  // Create the raster region for the whole screen
  X3D_Vex2D screen_v[] = {
    { offx, offy },
    { settings->screen_w - offx - 1, offy },
    { settings->screen_w - offx - 1, settings->screen_h - offy - 1 },
    { offx, settings->screen_h - offy - 1}
  };

  /*_Bool region = x3d_rasterregion_construct_from_points(
    &renderman->stack,
    &renderman->region,
    screen_v,
    4
  );*/

  //x3d_assert(region);

  //x3d_log(X3D_INFO, "Region (range=%d-%d)\n", renderman->region.rect.y_range.min, renderman->region.rect.y_range.max);

#ifndef __nspire__  
  // nspire has its zbuffer allocated with the screen
  renderman->zbuf = malloc(sizeof(int16) * screenman->w * screenman->h);
#endif
  
  
  renderman->render_hud_callback = NULL;
}

void test_clip(X3D_Polygon3D* poly, X3D_CameraObject* cam);


void x3d_render_id_buffer_polygon(X3D_Polygon3D* poly, uint32 id, X3D_CameraObject* cam) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
    }
    
    X3D_PolygonRasterAtt at = {
        .id_buffer = {
            .id = id,
        },
        .frustum = x3d_get_view_frustum(cam)
    };
    
    x3d_polygon3d_render_id_buffer(&rpoly, &at, cam);
}

extern X3D_Texture checkerboard;
extern X3D_Texture checkerboard2;

extern X3D_LightMapContext lightmap_context;

void x3d_render_texture_lightmap_polygon(X3D_Polygon3D* poly, X3D_Texture* tex, uint32 id, X3D_CameraObject* cam) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
        
        X3D_Vex2D v;
        x3d_planarprojection_project_point(&lightmap_context.proj[id], poly->v + i, &v);
        
        rpoly.v[i].lu = v.x;
        rpoly.v[i].lv = v.y;
    }
    
    if(rpoly.total_v == 4) {
        rpoly.v[0].uu = 0;
        rpoly.v[0].vv = 0;
        
        rpoly.v[1].uu = 63;
        rpoly.v[1].vv = 0;
        
        rpoly.v[2].uu = 63;
        rpoly.v[2].vv = 63;
        
        rpoly.v[3].uu = 0;
        rpoly.v[3].vv = 63;
    }
    else {
        X3D_Polygon2D p = { .v = alloca(1000) };
        x3d_polygon2d_construct(&p, rpoly.total_v, 128, 0);
        
        for(i = 0; i < rpoly.total_v; ++i) {
            rpoly.v[i].uu = p.v[i].x + 256;
            rpoly.v[i].vv = p.v[i].y + 256;
        }
    }
    
    
#if 0
    X3D_PolygonRasterAtt at = {
        .light_map = {
            .map = &lightmap_context.maps[id],
            .tex = tex
        },
        .frustum = x3d_get_view_frustum(cam)
    };
    
    x3d_polygon3d_render_texture_lightmap(&rpoly, &at, cam);
    
#else
    X3D_PolygonRasterAtt at = {
        .surface = {
            .tex = x3d_lightmapcontext_get_surface_for_level_face(&lightmap_context, id)
        },
        
        
        .frustum = x3d_get_view_frustum(cam)
    };
    
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    X3D_RenderManager* renderman = x3d_rendermanager_get();
    
    x3d_polygonrasteratt_set_screen(&at, screenman->buf, renderman->zbuf, screenman->w, screenman->h);
    x3d_polygon3d_render_texture_surface(&rpoly, &at, cam);
#endif
    
}

///////////////////////////////////////////////////////////////////////////////
/// Cleans up the render manager.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_rendermanager_cleanup(void) {
  free(x3d_rendermanager_get()->stack.base);
}


X3D_Level* global_level;
extern X3D_LineTexture3D logo;
extern X3D_LineTexture3D aperture;

void test_render_callback(X3D_CameraObject* cam);

void x3d_render_segment(X3D_SegRenderContext* context);

void render_cube(X3D_Vex3D pos, int16 size, X3D_SegRenderContext* context) {
    X3D_Prism3D prism = { .v = alloca(1000) };
    x3d_prism3d_construct(&prism, 4, size, size, (X3D_Vex3D_angle256) { 0, 0, 0 });
    
    x3d_prism3d_set_center(&prism, &pos);
    
    X3D_Polygon3D poly = { .v = alloca(1000) };
    
    uint16 i;
    for(i = 0; i < 6; ++i) {
        x3d_prism3d_get_face(&prism, i, &poly);
        
        if(context->render_context->render_type == X3D_RENDER_ID_BUFFER) {
            x3d_render_id_buffer_polygon(&poly, 0x7FFF, context->cam);
        }
        else {
            //x3d_render_textured_shaded_polygon(&poly, &checkerboard, context->cam);
        }
    }
}

void x3d_clip_polygon3d_to_frustum(X3D_Polygon3D* poly, X3D_Frustum* frustum, X3D_Polygon3D* dest) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    X3D_RasterPolygon3D clipped = { .v = alloca(1000) };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i)
        rpoly.v[i].v = poly->v[i];
    
    x3d_rasterpolygon3d_clip_to_frustum(&rpoly, frustum, &clipped);
    
    for(i = 0; i < clipped.total_v; ++i)
        dest->v[i] = clipped.v[i].v;
    
    dest->total_v = clipped.total_v;
}

void x3d_construct_frustum_from_clipped_polygon3d(X3D_Polygon3D* poly, X3D_Frustum* frustum, X3D_Frustum* dest, X3D_Vex3D* cam_pos) {
    X3D_Polygon3D clipped = { .v = alloca(1000) };
    
    x3d_clip_polygon3d_to_frustum(poly, frustum, &clipped);
    x3d_frustum_construct_from_polygon3d(dest, &clipped, cam_pos);
}

void x3d_render_segment_face(X3D_SegRenderContext* context, X3D_Prism3D* seg_geo, uint16 face) {
    X3D_LevelSegFace* face_att = x3d_levelsegment_get_face_attribute(context->level, context->seg, face);
    X3D_Vex3D cam_pos;
    x3d_object_pos(context->cam, &cam_pos);
        
    if(x3d_plane_point_distance(&face_att->plane, &cam_pos) < 0) return;
    
    if(face_att->connect_face != X3D_FACE_NONE) {
        if(x3d_segfaceid_seg(face_att->connect_face) != context->parent_seg_id) {
            X3D_SegRenderContext child_context = {
                .cam = context->cam,
                .seg = x3d_level_get_segmentptr(context->level, x3d_segfaceid_seg(face_att->connect_face)),
                .level = context->level,
                .parent_seg_id = context->seg->id,
                .render_context = context->render_context
            };
            
            x3d_render_segment(&child_context);
        }
    }
    else {
        X3D_Polygon3D temp = { .v = alloca(1000) };
        
        x3d_prism3d_get_face(seg_geo, face, &temp);
        
        if(context->render_context->render_type == X3D_RENDER_ID_BUFFER) {
            x3d_render_id_buffer_polygon(&temp, x3d_segfaceid_create(context->seg->id, face), context->cam);
        }
        else if(context->render_context->render_type == X3D_RENDER_LIGHTMAP) {
            //x3d_render_lightmap_polygon(&temp, x3d_segfaceid_create(context->seg->id, face), context->cam);
        }
        else if(context->render_context->render_type == X3D_RENDER_TEXTUER_LIGHTMAP) {          
            x3d_render_texture_lightmap_polygon(&temp, &checkerboard, x3d_segfaceid_create(context->seg->id, face), context->cam);
        }
                
        //==============
        if(face_att->wall_seg_start != X3D_FACE_NONE) {
            uint16* list = context->level->wall_segs.wall_segs + face_att->wall_seg_start;
            
            //x3d_log(X3D_INFO, "Total: %d", x3d_segfaceid_seg(list[1]));
            
            uint16 i;
            for(i = 0; i < list[0]; ++i) {
                X3D_SegRenderContext child_context = {
                    .cam = context->cam,
                    .seg = x3d_level_get_segmentptr(context->level, x3d_segfaceid_seg(list[i + 1])),
                    .level = context->level,
                    .parent_seg_id = context->seg->id,
                    .render_context = context->render_context
                };
                
                X3D_Prism3D p = { .v = alloca(1000) };
                x3d_levelsegment_get_geometry(context->level, child_context.seg, &p);
                
                X3D_Polygon3D poly = { .v = alloca(1000) };
                x3d_prism3d_get_face(&p, x3d_segfaceid_face(list[i + 1]), &poly);
                
                X3D_PolygonRasterVertex3D v[poly.total_v];
                uint16 j;
                
                for(j = 0; j < poly.total_v; ++j)
                    v[j].v = poly.v[j];
                
                X3D_RasterPolygon3D rpoly = { .v = v, .total_v = poly.total_v };
                X3D_PolygonRasterAtt att = {
                    .zfill = { 0x7F7F },
                    .frustum = x3d_get_view_frustum(context->cam)
                };
                
                
                x3d_polygon3d_render_zfill(&rpoly, &att, context->cam);
                
                x3d_render_segment(&child_context);
            }
        }
        
    }
}

void x3d_render_segment(X3D_SegRenderContext* context) {
    X3D_Prism3D prism = { .v = alloca(1000) };
    x3d_levelsegment_get_geometry(context->level, context->seg, &prism);
    
    uint16 i;
    for(i = 0; i < context->seg->base_v + 2; ++i) {
        x3d_render_segment_face(context, &prism, i);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// Renders the scene through a camera.
///
/// @param cam  - camera to render through
///
///////////////////////////////////////////////////////////////////////////////
void x3d_render(X3D_CameraObject* cam, X3D_RenderContext* render_context) {
    //x3d_renderer_draw_all_segments(global_level, cam, 31);

    uint16 seg = x3d_camera_get_seg_currently_in(cam, global_level);
    
    if(seg == 0xFFFF)
        return;
    
    X3D_SegRenderContext context = {
        .cam = cam,
        .seg = x3d_level_get_segmentptr(global_level, seg),
        .level = global_level,
        .parent_seg_id = 0xFFFF,
        .render_context = render_context
    };
    
    x3d_render_segment(&context);
    
    if(render_context->render_type == X3D_RENDER_TEXTUER_LIGHTMAP) {
        //x3d_screen_zbuf_visualize();
    }
    
    x3d_send_render_events_to_objects(cam);
    
    test_render_callback(cam);
    
    x3d_enginestate_next_step();
}

