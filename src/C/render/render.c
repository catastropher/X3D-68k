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
#include "X3D_segment.h"
#include "X3D_enginestate.h"
#include "X3D_clip.h"
#include "X3D_trig.h"
#include "X3D_collide.h"
#include "X3D_wallportal.h"
#include "X3D_portal.h"
#include "X3D_object.h"
#include "X3D_fastsqrt.h"
#include "X3D_polygon.h"

#include <stdio.h>


extern int16 render_mode;
extern uint16 geo_render_mode;


///////////////////////////////////////////////////////////////////////////////
/// Initializes the render manager.
///
/// @param settings - initialization settings
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_rendermanager_init(X3D_InitSettings* settings) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  // Initialize the render stack
  uint32 stack_size = 600000;
  void* render_stack_mem = malloc(stack_size);

  x3d_assert(render_stack_mem);

  x3d_stack_init(&renderman->stack, render_stack_mem, stack_size);
  
  // Reset segment face render callback
  renderman->segment_face_render_callback = NULL;
  

  int16 offx = 0, offy = 0;

  if(0) {
    offx = settings->screen_w / 4;
    offy = settings->screen_h / 4;
  }

  X3D_ScreenManager* screenman = x3d_screenmanager_get();

  screenman->w = settings->screen_w;
  screenman->h = settings->screen_h;

  // Create the raster region for the whole screen
  X3D_Vex2D screen_v[] = {
    { offx, offy },
    { settings->screen_w - offx - 1, offy },
    { settings->screen_w - offx - 1, settings->screen_h - offy - 1 },
    { offx, settings->screen_h - offy - 1}
  };

  _Bool region = x3d_rasterregion_construct_from_points(
    &renderman->stack,
    &renderman->region,
    screen_v,
    4
  );

  x3d_assert(region);

  x3d_log(X3D_INFO, "Region (range=%d-%d)\n", renderman->region.rect.y_range.min, renderman->region.rect.y_range.max);

#ifndef __nspire__  
  // nspire has its zbuffer allocated with the screen
  renderman->zbuf = malloc(sizeof(int16) * screenman->w * screenman->h);
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

///////////////////////////////////////////////////////////////////////////////
/// Renders any wall portals that are connected to a wall.
///
/// @param wall_id  - id of the wall
/// @param cam      - camera to render through
/// @param region   - clipping region to render into
///
/// @return The number of portals drawn.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_segment_render_wall_portals(X3D_SegFaceID wall_id, X3D_CameraObject* cam, X3D_RasterRegion* region) {
  uint16 portals[32];
  uint16 total_p = x3d_wall_get_wallportals(wall_id, portals);

  if(total_p > 0) {
    uint16 i;

    for(i = 0; i < total_p; ++i)
      x3d_wallportal_render(portals[i], cam, region, NULL);
  }

  return total_p;
}

///////////////////////////////////////////////////////////////////////////////
/// Generates rasteredges for a clipped polygon.
///
/// @param clip   - clipping context
/// @param stack  - stack to allocate rasteredges on
///
/// @return Nothing.
/// @note   This is part of the old clipping implementation and should not be
///         used in new code!
///////////////////////////////////////////////////////////////////////////////
void x3d_clipcontext_generate_rasteredges(X3D_ClipContext* clip, X3D_Stack* stack) {
  uint16 i;

  for(i = 0; i < clip->total_e; ++i) {
    uint16 a, b;

    a = clip->edge_pairs[i].val[0];
    b = clip->edge_pairs[i].val[1];
    
    int16 scale_a = 0x7FFF;//clip->depth_scale[a];
    int16 scale_b = 0x7FFF;//clip->depth_scale[b];
    
    if(clip->v3d[a].z < 15 || clip->v3d[b].z < 15) {
      scale_a = 0x7FFF;
      scale_b = 0x7FFF;
    }

    X3D_Vex3D temp_a = clip->v3d[a], temp_b = clip->v3d[b];
    X3D_Vex2D dest_a, dest_b;

    uint16 res = x3d_clip_line_to_near_plane(&temp_a, &temp_b, clip->v2d + a, clip->v2d + b, &dest_a, &dest_b, x3d_rendermanager_get()->near_z);

    if(!(res & EDGE_INVISIBLE)) {
      x3d_rasteredge_generate(clip->edges + i, dest_a, dest_b, clip->parent, clip->v3d[a].z, clip->v3d[b].z, stack, scale_a, scale_b);
      clip->edges[i].flags |= res;
    }
    else {
      clip->edges[i].flags = res;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs a segment face that's clipped to a clipping region.
///
/// @param context  - clipping context
/// @param face     - face id
/// @param dest     - pointer to pointer to dest clipping region
/// @param r        - parent clipping region
/// @param near_z   - distance to near plane
///
/// @return Nothing.
/// @note   *dest will be NULL if the region is totall invisible.
///////////////////////////////////////////////////////////////////////////////
void x3d_segment_construct_clipped_face(X3D_SegmentRenderContext* context, uint16 face, X3D_RasterRegion** dest, X3D_RasterRegion* r, int16 near_z) {
  X3D_Prism3D* prism = &context->seg->prism;            // Segment's prism
  uint16 edge_index[prism->base_v + 1];                 // Edge indexes for the raster region

  uint16 face_e = x3d_prism_face_edge_indexes(prism->base_v, face, edge_index);

  context->clip->edge_index = edge_index;
  context->clip->total_edge_index = face_e;
  
  context->clip->normal = &context->faces[face].plane.normal;

  _Bool use_new = x3d_rasterregion_construct_clipped(context->clip, r);

  if(context->faces[face].portal_seg_face != X3D_FACE_NONE && near_z <= 10) {
    *dest = context->parent;
  }
  else if(use_new) {
    *dest = r;
  }
  else {
    *dest = NULL;
  }
}


void x3d_set_texture(int16 id);

void x3d_segment_render_face_set_texture_uv(X3D_SegmentRenderContext* context, X3D_Polygon3D* p, uint16 face, uint16* u_dest, uint16* v_dest) {
  uint16 u[10] = { 0, 128, 128, 0 };
  uint16 v[10] = { 0, 0, 128, 128 };
  
  if(p->total_v == 8) {
    uint16 d;
    int16 r = 127;
    int16 cx = 128;
    int16 cy = 128;
    
    for(d = 0; d < 8; ++d) {
      u[d] = cx + (((int32)r * x3d_cos(d * 256 / 8 + ANG_45 + ANG_30 - 5)) >> 15);
      v[d] = cy + (((int32)r * x3d_sin(d * 256 / 8 + ANG_45 + ANG_30 - 5)) >> 15);
    }
  }
  
  if(context->seg_id == 0) {
    if(face != 1 && face != 0) {
      if(face != 8)
        x3d_set_texture(0);
      else
        x3d_set_texture(4);
    }
    else {
      
      x3d_set_texture(2);
    }
  }
  else if(context->seg_id <= 3) {
    x3d_set_texture(1);
  }

  uint16 i;
  for(i = 0; i < p->total_v; ++i) {
    u_dest[i] = u[i];
    v_dest[i] = v[i];
  }
}

void x3d_segment_render_face_attachments(X3D_SegmentRenderContext* context, uint16 face) {
  X3D_SegmentFaceAttachement* a = context->faces[face].attach;
  
  geo_render_mode = 1;
  
  while(a) {
    if(a->type == X3D_ATTACH_WALL_PORTAL) {
      X3D_RasterRegion* region = context->parent;
      X3D_RenderManager* renderman = x3d_rendermanager_get();
      
      // Save the stack pointer so we can free any allocations made later
      void* stack_ptr = x3d_stack_save(&renderman->stack);

      X3D_Polygon3D poly = { .v = alloca(1000) };
      
      X3D_Segment* other_seg = x3d_segmentmanager_load(x3d_segfaceid_seg(a->flags));
      x3d_prism3d_get_face(&other_seg->prism, x3d_segfaceid_face(a->flags), &poly);
      
      
      // The wall portal to be rendered (using the temporary global implementation)
      // Outline of the portal projected onto the screen
      X3D_Vex2D v2d[poly.total_v];

      // 3D points that make up the outline and have been rotated relative to the
      // camera, but haven't been projected yet
      X3D_Vex3D v3d[poly.total_v];

      // Transform the points so they are relative to the camera
      x3d_camera_transform_points(context->cam, poly.v, poly.total_v,
        v3d, v2d);

      uint16 i;


      X3D_RasterEdge edges[poly.total_v + 1];

      X3D_Pair edge_pair[poly.total_v];

      // Construct the portal's raster region, which has to be clipped against the
      // parent raster region
      for(i = 0; i < poly.total_v; ++i) {
        uint16 a, b;

        a = i;
        b = i + 1 < poly.total_v ? i + 1 : 0;

        edge_pair[i].val[0] = a;
        edge_pair[i].val[1] = b;

        X3D_Vex3D temp_a = v3d[a], temp_b = v3d[b];
        X3D_Vex2D dest_a, dest_b;

        uint16 flags = x3d_clip_line_to_near_plane(&temp_a, &temp_b, v2d + a, v2d + b, &dest_a, &dest_b, 10);

        edges[i].flags = 0;

        if(!(flags & EDGE_INVISIBLE))
          x3d_rasteredge_generate(edges + i, dest_a, dest_b, region, v3d[a].z, v3d[b].z, &renderman->stack, 0x7FFF, 0x7FFF);

        edges[i].flags |= flags;
      }

      X3D_RasterRegion clipped_region;
      uint16 edge_index[poly.total_v+ 1];

      for(i = 0; i < poly.total_v; ++i)
        edge_index[i] = i;

      X3D_ClipContext clip = {
        .stack = &renderman->stack,
        .parent = region,
        .edges = edges,
        .total_e = poly.total_v,
        .v3d = v3d,
        .v2d = v2d,
        .edge_pairs = edge_pair,
        .edge_index = edge_index,
        .total_edge_index = poly.total_v
      };
      
      _Bool use_new = x3d_rasterregion_construct_clipped(&clip, &clipped_region);
      
      X3D_Portal port;

      //x3d_portal_set_fill(&port, 31);

      if(0) {//0 <= 10) {
        port.region = region;
      }
      else if(use_new) {
        port.region = &clipped_region;
      }
      else {
        port.region = NULL;
      }
      
      if(port.region != NULL) {
        
      //x3d_segment_construct_clipped_face(context, face, &clipped, &r, 15);
        x3d_segment_render(x3d_segfaceid_seg(a->flags), context->cam, 31, port.region, context->step, x3d_segfaceid_face(a->flags));
        //x3d_rasterregion_update(port.region);
        //x3d_rasterregion_fill_texture(port.region, 15);
      }
      
      x3d_stack_restore(&renderman->stack, stack_ptr);
    }
    
    a = a->next;
  }
}

void x3d_segment_render_face(X3D_SegmentRenderContext* context, uint16 face) {
  x3d_segment_render_wall_portals(x3d_segfaceid_create(context->seg_id, face), context->cam, context->parent);
  
  X3D_Polygon3D p = {
    .v = alloca(1000)
  };
  
  x3d_prism3d_get_face(&context->seg->prism, face, &p);

  x3d_segment_render_face_attachments(context, face);
  
  uint16 u[10];
  uint16 v[10];
  x3d_segment_render_face_set_texture_uv(context, &p, face, u, v);
  
  geo_render_mode = 0;
  
  X3D_Vex3D normal[10];
  
  if(1) {
    if(1) {//!x3d_key_down(X3D_KEY_15) || ((face == 5) && (context->seg_id == 4)))
      x3d_polygon3d_render(&p, context->cam, context->parent, 31, normal, u, v);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Renders the faces of a segment.
///
/// @param context  - segment rendering context
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_segment_render_faces(X3D_SegmentRenderContext* context) {
  uint16 i;
  
  X3D_Prism3D* prism = &context->seg->prism;

  // Render the connecting segments
  for(i = 0; i < x3d_prism3d_total_f(prism->base_v); ++i) {
    if(i != context->portal_face) {
      int16 dist = x3d_plane_dist(&context->faces[i].plane, &context->cam->pseduo_pos);
      
      if(dist > 0) {
        void* stack_ptr = x3d_stack_save(&context->renderman->stack);
        
        x3d_set_texture(1);
        
        if(context->faces[i].portal_seg_face == X3D_FACE_NONE) {
          x3d_segment_render_face(context, i);
        }
        else {
          X3D_Portal portal;
          X3D_RasterRegion r;
          
          x3d_segment_construct_clipped_face(context, i, &portal.region, &r, dist);
          
          if(portal.region) {
            uint16 seg_id = x3d_segfaceid_seg(context->faces[i].portal_seg_face);
            uint16 seg_face = x3d_segfaceid_face(context->faces[i].portal_seg_face);
            
            x3d_portal_render(&portal);
            x3d_segment_render(seg_id, context->cam, 0, portal.region, context->step, seg_face);
          }
        }

        x3d_stack_restore(&context->renderman->stack, stack_ptr);
      }
    }
  }
}

int16 depth = 0;

int16 x3d_t_clip(int16 start, int16 end, uint16 scale);

void x3d_segment_render_door(X3D_SegmentRenderContext* context) {
  if(context->seg->door_data->mode == X3D_DOOR_OPENING) {
    context->seg->door_data->door_open = X3D_MIN((int32)context->seg->door_data->door_open + context->seg->door_data->open_speed, 0x7FFF);
  }
  else if(context->seg->door_data->mode == X3D_DOOR_CLOSING) {
    context->seg->door_data->door_open = X3D_MAX((int32)context->seg->door_data->door_open - context->seg->door_data->open_speed, 0);
  }
  
  uint16 i;
  X3D_Prism3D* prism = &context->seg->prism;
  
  X3D_Plane plane;
  X3D_Polygon3D poly = { .v = alloca(1000) };
  X3D_Polygon3D poly2 = { .v = alloca(1000) };
  
  x3d_prism3d_get_face(prism, 3, &poly);
  x3d_polygon3d_calculate_plane(&poly, &plane);
  
  X3D_Vex3D center;
  x3d_prism3d_get_face(prism, X3D_BASE_A, &poly);
  x3d_polygon3d_center(&poly, &center);
  
  plane.d = x3d_vex3d_fp0x16_dot(&center, &plane.normal);
  
  uint16 u[10];
  uint16 v[10];
  
  uint16 new_u[12];
  uint16 new_v[12];

  X3D_Vex3D move_right = x3d_vex3d_neg(&plane.normal);
  
  uint16 d;
  
  uint16 clip[10];
  
  X3D_Polygon3D side_poly = { .v = alloca(1000) };
  
  int16 open = x3d_t_clip(0, context->seg->door_data->max_open, context->seg->door_data->door_open);
  
  #if 1
  for(i = 0; i < prism->base_v + 2; ++i) {
    if(i == 2 || i == 4 ) {
      geo_render_mode = 1;
      x3d_prism3d_get_face(prism, i, &poly);
      x3d_segment_render_face_set_texture_uv(context, &poly, i, u, v);
      
      X3D_Plane p_plane;
      x3d_polygon3d_calculate_plane(&poly, &p_plane);
      x3d_polygon3d_render(&poly, context->cam, context->parent, 31, &p_plane.normal, u, v);
    }
    else if(context->faces[i].portal_seg_face != X3D_FACE_NONE && context->seg->door_data->door_open != X3D_DOOR_CLOSED) {
      X3D_Portal portal;
      X3D_RasterRegion r;
      
      x3d_segment_construct_clipped_face(context, i, &portal.region, &r, 15);
      
      if(portal.region) {
        uint16 seg_id = x3d_segfaceid_seg(context->faces[i].portal_seg_face);
        uint16 seg_face = x3d_segfaceid_face(context->faces[i].portal_seg_face);
        
        x3d_portal_render(&portal);
        x3d_segment_render(seg_id, context->cam, 0, portal.region, context->step, seg_face);
      }
    }
  }
#endif
  
  for(d = 0; d < 2; ++d) {
    side_poly.total_v = 0;
    
    for(i = 0; i < prism->base_v + 2; ++i) {
      if(i > X3D_BASE_B)
        continue;
      
      if(1) {
        geo_render_mode = 1;
        x3d_prism3d_get_face(prism, i, &poly);
        x3d_segment_render_face_set_texture_uv(context, &poly, i, u, v);
        _Bool c = x3d_polygon3d_clip_to_plane(&poly, &poly2, &plane, u, v, new_u, new_v, clip);
        
        X3D_Plane p_plane;
        x3d_polygon3d_calculate_plane(&poly, &p_plane);
        
        //p_plane.normal = x3d_vex3d_neg(&p_plane.normal);
        //p_plane.d = -p_plane.d

        x3d_polygon3d_translate_normal(&poly2, &move_right, open);
        
        if(1) {
          side_poly.v[side_poly.total_v++] = poly2.v[clip[0]];
          side_poly.v[side_poly.total_v++] = poly2.v[clip[1]];
          
          //x3d_log(X3D_INFO, "%d %d", clip[0], clip[1]);
        }
        else {
          side_poly.v[side_poly.total_v++] = poly2.v[clip[1]];
          side_poly.v[side_poly.total_v++] = poly2.v[clip[0]];
        }
        
        x3d_polygon3d_render(&poly2, context->cam, context->parent, 31, &p_plane.normal, new_u, new_v);
      }
    }
    
    
    X3D_Vex3D norm = plane.normal;
    
    geo_render_mode = 1;
    
    x3d_prism3d_get_face(prism, i, &poly);
    x3d_segment_render_face_set_texture_uv(context, &poly, d, u, v);
    
    if(1) {
      x3d_polygon3d_render(&side_poly, context->cam, context->parent, 31, &norm, u, v);
    }
    
    plane.normal = x3d_vex3d_neg(&plane.normal);
    move_right = x3d_vex3d_neg(&move_right);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Renders a segment.
///
/// @param id           - seg id
/// @param cam          - camera to render through
/// @param color        - color of the segment (should be removed as not needed)
/// @param region       - parent clipping region
/// @param step         - current engine step
/// @param portal_face  - face the segment is being rendered through
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region, uint16 step, uint16 portal_face) {
  // Make sure we don't blow the stack from recursing too deep
  if(depth >= 15)
    return;
  
  ++depth;
  
  X3D_RenderManager* renderman      = x3d_rendermanager_get();
  void* stack_save                  = x3d_stack_save(&renderman->stack);
  X3D_Segment* seg                  = x3d_segmentmanager_load(id);
  X3D_Prism3D* prism                = &seg->prism;
  uint16 total_e                    = seg->prism.base_v * 3;
  
  X3D_ClipContext clip = {
    .stack        = &renderman->stack,
    .parent       = region,
    .edges        = X3D_STACK_ALLOC_TYPE(&renderman->stack, X3D_RasterEdge, total_e + 5),
    .total_e      = total_e,
    .v3d          = X3D_STACK_ALLOC_TYPE(&renderman->stack, X3D_Vex3D, prism->base_v * 2),
    .v2d          = X3D_STACK_ALLOC_TYPE(&renderman->stack, X3D_Vex2D, prism->base_v * 2),
    .edge_pairs   = X3D_STACK_ALLOC_TYPE(&renderman->stack, X3D_Pair, total_e + 1),
    .depth_scale  = NULL,
    .seg = seg
  };
  
  X3D_SegmentRenderContext context = {
    .seg          = seg,
    .seg_id       = id,
    .faces        = x3d_uncompressedsegment_get_faces(seg),
    .renderman    = renderman,
    .parent       = region,
    .cam          = cam,
    .clip         = &clip,
    .step         = step,
    .list         = NULL,
    .portal_face  = portal_face
  };
  
  X3D_Vex3D cam_pos, cam_dir;
  x3d_object_pos(cam, &cam_pos);
  x3d_camera_transform_points(cam, prism->v, prism->base_v * 2, clip.v3d, clip.v2d);
  x3d_dynamicobject_forward_vector(&cam->base, &cam_dir);

  x3d_prism_get_edge_pairs(prism->base_v, clip.edge_pairs);
  x3d_clipcontext_generate_rasteredges(&clip, &renderman->stack);
  
  if(x3d_segment_is_door(seg))
    x3d_segment_render_door(&context);
  else
    x3d_segment_render_faces(&context);

  if(id == 0) {
    x3d_set_texture(3);
    x3d_cube_render((X3D_Vex3D) { 150, 75, 150 }, 75, cam, region);
  }

  x3d_stack_restore(&renderman->stack, stack_save);

  --depth;  
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the surface normal of a point on a sphere.
///
/// @param center - center of the sphere
/// @param v      - point on the sphere
/// @param dest   - dest
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_sphere_normal(X3D_Vex3D* center, X3D_Vex3D* v, X3D_Vex3D* dest) {
  dest->x = v->x - center->x;
  dest->y = v->y - center->y;
  dest->z = v->z - center->z;
  
  x3d_vex3d_fp0x16_normalize(dest);
}

///////////////////////////////////////////////////////////////////////////////
/// Renders a sphere.
///
/// @param center   - center of the sphere
/// @param r        - radius
/// @param steps    - number of discrete steps used to polygonize the sphere
/// @param c        - color (unused)
/// @param cam      - camera to render through
/// @param region   - clipping region
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_sphere_render(X3D_Vex3D center, int16 r, int16 steps, X3D_Color c, X3D_CameraObject* cam, X3D_RasterRegion* region) {
  int16 rad[steps + 1];
  
  int16 i;
  for(i = 0; i <= steps; ++i) {
    int16 rr = -r + (r * 2 / steps) * i;
    rad[i] = x3d_fastsqrt(r * r - rr * rr);
  }
  
  X3D_Vex3D v1[steps];
  X3D_Vex3D v2[steps];
  
  X3D_Vex3D* top = v1;
  X3D_Vex3D* bottom = v2;
  
  for(i = 0; i < steps; ++i) {
    top[i].x = center.x;
    top[i].y = center.y - r;
    top[i].z = center.z;
  }
  
  X3D_Mat3x3 mat;
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };//x3d_enginestate_get_step(), x3d_enginestate_get_step(), 0 };
  x3d_mat3x3_construct(&mat, &angle);
  
  int16 d;
  for(i = 0; i <= steps; ++i) {
    ufp8x8 angle = 0;
    
    for(d = 0; d < steps; ++d) {
      bottom[d].x = (((int32)rad[i] * x3d_cos(angle >> 8)) >> 15) + center.x;
      bottom[d].z = (((int32)rad[i] * x3d_sin(angle >> 8)) >> 15) + center.z;
      bottom[d].y = top[0].y + r * 2 / steps;
      
      X3D_Vex3D rot;
      x3d_vex3d_int16_rotate(&rot, bottom + d, &mat);
      bottom[d] = rot;
      
      angle += (uint16)65535 / steps;
    }
    
    for(d = 0; d < steps; ++d) {
      uint16 next = (d + 1 < steps ? d + 1 : 0);
      X3D_Vex3D v[4] = {
        top[d],
        top[next],
        bottom[next],
        bottom[d]
      };
      
      X3D_Polygon3D p = {
        .v = v,
        .total_v = 4
      };
      
      X3D_Plane plane;
      
      //x3d_plane_construct(&plane, p.v + 0, p.v + 1, p.v + 2);
      
      X3D_Vex3D norm[4];
      
      int16 k;
      for(k = 0; k < 4; ++k) {
        if(render_mode != 0) {
          x3d_sphere_normal(&center, p.v + k, norm + k);
        }
        else {
          x3d_plane_construct(&plane, p.v + 1, p.v + 2, p.v + 0);
          norm[k] = plane.normal;
        }
      }
      
      //if(i == 3 && d == 7)
      //if(i == 2 && d == 2)
      
      uint16 uu[4] = { 0, 63, 63, 0 };
      uint16 vv[4] = { 0, 0, 63, 63 };
      
      x3d_polygon3d_render(&p, cam, region, c, norm, uu, vv);
    }
    
    X3D_SWAP(top, bottom);
  }
  
}

///////////////////////////////////////////////////////////////////////////////
/// Renders a cube.
///
/// @param center - center of the cube
/// @param w      - width/height of the cube
/// @param cam    - camera to render through
/// @param region - parent clipping region
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_cube_render(X3D_Vex3D center, int16 w, X3D_CameraObject* cam, X3D_RasterRegion* region) {
//  return;
  X3D_Prism3D* prism = alloca(1000);
  int16 steps = 4;

  geo_render_mode = 1;
  
  uint8 angle = x3d_enginestate_get_step() * 4;
  
  x3d_prism3d_construct(prism, steps, w, w, (X3D_Vex3D_angle256) { angle, angle, 0 }); //x3d_enginestate_get_step() * 2, x3d_enginestate_get_step() * 2, 0 });
  X3D_Polygon3D p = {
    .v = alloca(1000)
  };
  
  X3D_Vex3D norm[10] = {
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 },
    { 0x7FFF, 0, 0 }
  };
  
  uint16 i;
  for(i = 0; i < steps * 2; ++i) {
    prism->v[i].x += center.x;
    prism->v[i].y += center.y;
    prism->v[i].z += center.z;
  }
  
  x3d_prism3d_get_face(prism, 1, &p);
  x3d_polygon3d_scale(&p, 256 + 128);
  //x3d_prism3d_set_face(prism, 1, &p);

  //p.v[0] = (X3D_Vex3D) { 100, 100, 100 };
  //p.v[1] = (X3D_Vex3D) { -100, -150, 100 };
  //p.v[2] = (X3D_Vex3D) { 100, -100, 100 };
  
  //p.total_v = 3;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  for(i = 0; i < steps + 2; ++i) {
    x3d_prism3d_get_face(prism, i, &p);

    uint16 u[4] = { 0, 128, 128, 0 };
    uint16 v[4] = { 0, 0, 128, 128 };
    
    X3D_Plane plane;
    x3d_polygon3d_calculate_plane(&p, &plane);
    
    if(x3d_plane_dist(&plane, &cam_pos) > 0)
      continue;
    
    x3d_polygon3d_render(&p, cam, region, 0, norm, u, v);
  }
}

#include <SDL/SDL.h>

///////////////////////////////////////////////////////////////////////////////
/// Renders the scene through a camera.
///
/// @param cam  - camera to render through
///
///////////////////////////////////////////////////////////////////////////////
void x3d_render(X3D_CameraObject* cam) {
  /// @todo Pseduo position isn't needed anymore since the portal implementation was upgraded
  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);

  static uint32 start = 0;
  static uint16 frames = 0;
  
  x3d_screen_zbuf_clear();
  
  depth = 0;
  x3d_segment_render(cam->base.base.seg, cam, 0, &x3d_rendermanager_get()->region, x3d_enginestate_get_step(), 0xFFFF);

  // Draw the crosshair
  int16 cx = x3d_screenmanager_get()->w / 2;
  int16 cy = x3d_screenmanager_get()->h / 2;

  x3d_screen_draw_pix(cx, cy - 1, 0xFFFF);
  x3d_screen_draw_pix(cx, cy + 1, 0xFFFF);
  x3d_screen_draw_pix(cx - 1, cy, 0xFFFF);
  x3d_screen_draw_pix(cx + 1, cy, 0xFFFF);
  
  static int32 fps = 0;

  // Update FPS counter
  if(++frames == 10) {
    int32 time = (SDL_GetTicks() - start);
    if(time != 0)
      fps = 1000000 / time;
    else
      fps = 100000;
    
    frames = 0;
    
    start = SDL_GetTicks();
  }
  
  x3d_screen_draw_uint32(fps, 0, 0, 31);  
}

