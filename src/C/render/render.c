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

void x3d_render_flat_shaded_polygon(X3D_Polygon3D* poly, X3D_Color c, X3D_CameraObject* cam) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
    }
    
    X3D_PolygonRasterAtt at = {
        .flat = {
            .color = c
        }
    };
    
    x3d_polygon3d_render_flat(&rpoly, &at, cam);
}

void x3d_render_id_buffer_polygon(X3D_Polygon3D* poly, uint32 id, X3D_CameraObject* cam) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
    }
    
    X3D_PolygonRasterAtt at = {
        .id_buffer = {
            .id = id
        }
    };
    
    x3d_polygon3d_render_id_buffer(&rpoly, &at, cam);
}

void x3d_render_gouraud_shaded_polygon(X3D_Polygon3D* poly, X3D_Color c, X3D_CameraObject* cam) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    float intensity[10];
    
    uint16 j;
    for(j = 0; j < rpoly.total_v; ++j) {
        intensity[j] = ((float)1 / rpoly.total_v) * (j);
    }
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
        rpoly.v[i].intensity = intensity[i] * 32767;
    }
    
    X3D_PolygonRasterAtt at = {
        .flat = {
            .color = c
        }
    };
    
    x3d_polygon3d_render_gouraud(&rpoly, &at, cam);
    
    if(x3d_key_down(X3D_KEY_15)) {
        x3d_screen_flip();
        SDL_Delay(500);
        x3d_screen_flip();
    }
}

extern X3D_Texture checkerboard;
extern X3D_Texture checkerboard2;

void x3d_render_textured_shaded_polygon(X3D_Polygon3D* poly, X3D_Texture* tex, X3D_CameraObject* cam) {
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    float intensity[10];
    
    uint16 j;
    for(j = 0; j < rpoly.total_v; ++j) {
        intensity[j] = ((float)1 / rpoly.total_v) * (j);
    }
    
    if(poly->total_v == 4) {
        intensity[0] = 0;
        intensity[1] = 0;
        intensity[2] = 0;
        intensity[3] = 1;
    }
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
        rpoly.v[i].intensity = intensity[i] * 32767;
    }
    
    rpoly.v[0].uu = 0;
    rpoly.v[0].vv = 0;
    
    rpoly.v[1].uu = 63;
    rpoly.v[1].vv = 0;
    
    rpoly.v[2].uu = 63;
    rpoly.v[2].vv = 63;
    
    rpoly.v[3].uu = 0;
    rpoly.v[3].vv = 63;
    
    X3D_PolygonRasterAtt at = {
        .texture = {
            .texture = tex
        }
    };
    
    x3d_polygon3d_render_texture(&rpoly, &at, cam);
}

extern X3D_LightMapContext lightmap_context;

void x3d_render_lightmap_polygon(X3D_Polygon3D* poly, uint32 id, X3D_CameraObject* cam) {
    return;
    X3D_RasterPolygon3D rpoly = { .v = alloca(1000), .total_v = poly->total_v };
    
    float intensity[10];
    
    
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        rpoly.v[i].v = poly->v[i];
        
        X3D_Vex2D v;
        x3d_planarprojection_project_point(&lightmap_context.proj[id], poly->v + i, &v);
        
        rpoly.v[i].uu = v.x;
        rpoly.v[i].vv = v.y;
    }
    
    X3D_PolygonRasterAtt at = {
        .light_map = {
            .map = &lightmap_context.maps[id]
        }
    };
    
    x3d_polygon3d_render_lightmap(&rpoly, &at, cam);
}

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
    
    X3D_PolygonRasterAtt at = {
        .light_map = {
            .map = &lightmap_context.maps[id],
            .tex = tex
        }
    };
    
    x3d_polygon3d_render_texture_lightmap(&rpoly, &at, cam);
}

void x3d_renderer_draw_segment_wireframe(X3D_Level* level, X3D_LEVEL_SEG seg_id, X3D_CameraObject* cam, X3D_Color color) {
  X3D_LevelSegment* seg = x3d_level_get_segmentptr(level, seg_id);
  X3D_Prism3D prism = { .v = alloca(1000) };
  
  x3d_levelsegment_get_geometry(level, seg, &prism);
  X3D_Polygon3D* temp = x3d_polygon3d_temp();
  
  X3D_Color colors[] = {
    x3d_rgb_to_color(255, 255, 255),
    x3d_rgb_to_color(255, 0, 255),
    x3d_rgb_to_color(255, 0, 0),
    x3d_rgb_to_color(0, 255, 0),
    x3d_rgb_to_color(0, 0, 255),
    x3d_rgb_to_color(255, 255, 0),
    x3d_rgb_to_color(0, 255, 255),
    x3d_rgb_to_color(128, 128, 128),
    x3d_rgb_to_color(255, 128, 255),
    x3d_rgb_to_color(128, 128, 255)
  };
  
  uint16 i;
  for(i = 0; i < prism.base_v + 2; ++i) {
      if(x3d_levelsegment_get_face_attribute(level, seg, i)->connect_face != X3D_FACE_NONE) continue;
      
    x3d_prism3d_get_face(&prism, i, temp);
    
    if(temp->total_v != 4)
        x3d_render_gouraud_shaded_polygon(temp, colors[i], cam);
    else
        x3d_render_textured_shaded_polygon(temp, &checkerboard, cam);
  }
  
  //x3d_prism3d_render_wireframe(&prism, cam, color);
}

void x3d_renderer_draw_all_segments(X3D_Level* level, X3D_CameraObject* cam, X3D_Color color) {
  uint16 i;
  for(i = 0; i < level->segs.total; ++i)
    x3d_renderer_draw_segment_wireframe(level, i, cam, color);
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
            x3d_render_lightmap_polygon(&temp, x3d_segfaceid_create(context->seg->id, face), context->cam);
        }
        else if(context->render_context->render_type == X3D_RENDER_TEXTUER_LIGHTMAP) {          
            if(temp.total_v != 4) {
                if(context->seg->id == 1 || 1) {
                    x3d_render_texture_lightmap_polygon(&temp, &checkerboard, x3d_segfaceid_create(context->seg->id, face), context->cam);
                }
                else {
                    //x3d_render_lightmap_polygon(&temp, x3d_segfaceid_create(context->seg->id, face), context->cam);
                }
            }
                //
                //x3d_render_gouraud_shaded_polygon(&temp, x3d_rgb_to_color(32, 32, 32), context->cam);
            else
                x3d_render_texture_lightmap_polygon(&temp, &checkerboard, x3d_segfaceid_create(context->seg->id, face), context->cam);
        }
        else {
            if(temp.total_v != 4)
                x3d_render_gouraud_shaded_polygon(&temp, x3d_rgb_to_color(32, 32, 32), context->cam);
            else
                x3d_render_textured_shaded_polygon(&temp, &checkerboard, context->cam);
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
    
    if(context->seg->id == 0) {
        X3D_Ray3D ray = {{ x3d_vex3d_make(-150, 50, 400), x3d_vex3d_make(100, 0, 400) }};
        
        int16 steps = 50;
        
        X3D_Vex3D v;
        x3d_ray3d_interpolate(&ray, 0, &v);//32767 * (x3d_enginestate_get_step() % steps) / steps, &v);
        
        render_cube(v, 50, context);
        
        render_cube(x3d_vex3d_make(-175, 100, 425), 25, context);
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
        
    X3D_SegRenderContext context = {
        .cam = cam,
        .seg = x3d_level_get_segmentptr(global_level, 0),
        .level = global_level,
        .parent_seg_id = 0xFFFF,
        .render_context = render_context
    };
    
    x3d_render_segment(&context);
    
    x3d_send_render_events_to_objects(cam);
    
    test_render_callback(cam);
    
    x3d_enginestate_next_step();
}

