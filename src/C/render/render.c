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

#include <stdio.h>

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

  screenman->w = settings->screen_w;
  screenman->h = settings->screen_h;

#ifndef __nspire__  
  // nspire has its zbuffer allocated with the screen
  renderman->zbuf = malloc(sizeof(int16) * screenman->w * screenman->h);
#endif
  
  
  renderman->render_hud_callback = NULL;
}

extern X3D_Texture checkerboard;
extern X3D_Texture checkerboard2;

///////////////////////////////////////////////////////////////////////////////
/// Cleans up the render manager.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_rendermanager_cleanup(void) {
  free(x3d_rendermanager_get()->stack.base);
}


X3D_Level* global_level;

void test_render_callback(X3D_CameraObject* cam);

void x3d_render_segment(X3D_SegRenderContext* context);

static inline _Bool x3d_face_connects_to_parent_segment(X3D_LevelSegFace* face, X3D_SegRenderContext* context) {
    return x3d_segfaceid_seg(face->connect_face) == context->parent_seg_id;
}

static inline void x3d_render_segment_connected_to_face(X3D_LevelSegFace* face, X3D_SegRenderContext* context) {
    X3D_SegRenderContext child_context = {
        .cam = context->cam,
        .seg = x3d_level_get_segmentptr(context->level, x3d_segfaceid_seg(face->connect_face)),
        .level = context->level,
        .parent_seg_id = context->seg->id,
        .render_context = context->render_context
    };
    
    x3d_render_segment(&child_context);
}

static inline _Bool x3d_camera_is_on_visible_side_of_segment_face(X3D_CameraObject* cam, X3D_LevelSegFace* face) {
    X3D_Vex3D cam_pos;
    x3d_object_pos(cam, &cam_pos);
    
    return x3d_plane_point_is_on_normal_facing_side(&face->plane, &cam_pos);
}

void x3d_render_face(X3D_SegRenderContext* context, X3D_Prism3D* seg_geo, uint16 face_id) {
    X3D_Polygon3D face_geo = { .v = alloca(1000) };
    
    x3d_prism3d_get_face(seg_geo, face_id, &face_geo);
    
    if(context->render_context->render_type == X3D_RENDER_ID_BUFFER) {
        x3d_render_id_buffer_polygon(&face_geo, x3d_segfaceid_create(context->seg->id, face_id), context->cam);
    }
    else if(context->render_context->render_type == X3D_RENDER_TEXTUER_LIGHTMAP) {          
        x3d_render_texture_lightmap_polygon(&face_geo, &checkerboard, x3d_segfaceid_create(context->seg->id, face_id), context->cam);
    }
}

static inline void x3d_render_segments_attached_to_face(X3D_LevelSegFace* face, X3D_SegRenderContext* context) {
    uint16 total_segments_attached_to_face = x3d_levelsegface_total_segments_attached(face, context->level);
    uint16* segments_attached_to_face = x3d_levelsegface_attached_segments(face, context->level);
    
    for(uint16 i = 0; i < total_segments_attached_to_face; ++i) {
        X3D_SegRenderContext child_context = {
            .cam = context->cam,
            .seg = x3d_level_get_segmentptr(context->level, x3d_segfaceid_seg(segments_attached_to_face[i])),
            .level = context->level,
            .parent_seg_id = context->seg->id,
            .render_context = context->render_context
        };
        
        X3D_Prism3D p = { .v = alloca(1000) };
        x3d_levelsegment_get_geometry(context->level, child_context.seg, &p);
        
        X3D_Polygon3D poly = { .v = alloca(1000) };
        x3d_prism3d_get_face(&p, x3d_segfaceid_face(segments_attached_to_face[i]), &poly);
        
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

void x3d_render_segment_face(X3D_SegRenderContext* context, X3D_Prism3D* seg_geo, uint16 face_id) {
    X3D_LevelSegFace* face = x3d_levelsegment_get_face_attribute(context->level, context->seg, face_id);
        
    if(!x3d_camera_is_on_visible_side_of_segment_face(context->cam, face))
        return;
    
    if(x3d_level_segment_face_has_attached_segment(face)) {
        if(!x3d_face_connects_to_parent_segment(face, context))
            x3d_render_segment_connected_to_face(face, context);
    }
    else {
        x3d_render_face(context, seg_geo, face_id);
    
        if(x3d_levelsegface_has_segments_attached(face))
            x3d_render_segments_attached_to_face(face, context);
    }
}

void x3d_render_segment(X3D_SegRenderContext* context) {
    X3D_Prism3D prism = { .v = alloca(1000) };
    x3d_levelsegment_get_geometry(context->level, context->seg, &prism);
    
    for(uint16 i = 0; i < context->seg->base_v + 2; ++i)
        x3d_render_segment_face(context, &prism, i);
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

