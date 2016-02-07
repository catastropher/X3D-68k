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

int16 line_count;

int16 x3d_scale_by_depth(int16 value, int16 depth, int16 min_depth, int16 max_depth) {  
  if(x3d_rendermanager_get()->wireframe)
    return value;
  
  if(depth > max_depth)
    return 0;
  
  return value - ((int32)value * (depth - min_depth) / (max_depth - min_depth));
}

X3D_Color x3d_color_scale_by_depth(X3D_Color color, int16 depth, int16 min_depth, int16 max_depth) {
  uint8 r, g, b;
  x3d_color_to_rgb(color, &r, &g, &b);
  
  return color;
  
  return x3d_rgb_to_color(
    x3d_scale_by_depth(r, depth, min_depth, max_depth),
    x3d_scale_by_depth(g, depth, min_depth, max_depth),
    x3d_scale_by_depth(b, depth, min_depth, max_depth)                          
  );
}

///////////////////////////////////////////////////////////////////////////////
/// Clips a line to a raster region and draws it, if it's visible.
///
/// @param x1     - first x coordinate
/// @param y1     - first y coordinate
/// @param x2     - second x coordinate
/// @param y2     - second y coordinate
/// @param color  - color of the line
/// @param region - region to clip against
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_draw_clipped_line(int16 x1, int16 y1, int16 x2, int16 y2, int16 depth1, int16 depth2, X3D_Color color, X3D_RasterRegion* region) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  X3D_Vex2D v1 = { x1, y1 };
  X3D_Vex2D v2 = { x2, y2 };

  if(depth1 > 1500 && depth2 > 1500)
    return;

  
  if(x3d_rasterregion_clip_line(region, &renderman->stack, &v1, &v2)) {
    X3D_Color new1 = x3d_color_scale_by_depth(color, depth1, 10, 1500);
    X3D_Color new2 = x3d_color_scale_by_depth(color, depth2, 10, 1500);
    
    x3d_screen_draw_line_grad(v1.x, v1.y, v2.x, v2.y, new1, new2);
  }
  
  ++line_count;
}

void x3d_displaylinelist_add(X3D_DisplayLineList* list, X3D_Vex2D a, int16 a_depth, X3D_Vex2D b, int16 b_depth, X3D_Color color) {
  list->lines[list->total_l].v[0].x = a.x;
  list->lines[list->total_l].v[0].y = a.y;
  list->lines[list->total_l].v[0].z = a_depth;
  
  list->lines[list->total_l].v[1].x = b.x;
  list->lines[list->total_l].v[1].y = b.y;
  list->lines[list->total_l].v[1].z = b_depth;
  
  list->lines[list->total_l].color = color;
  
  ++list->total_l;
}

void x3d_displaylinelist_render(X3D_DisplayLineList* list, X3D_RasterRegion* region) {
  uint16 i;
  for(i = 0; i < list->total_l; ++i) {
    x3d_draw_clipped_line(
      list->lines[i].v[0].x,
      list->lines[i].v[0].y,
      list->lines[i].v[1].x,
      list->lines[i].v[1].y,
      list->lines[i].v[0].z,
      list->lines[i].v[1].z,
      list->lines[i].color,
      region
    );
  }
}

void x3d_rasteredge_list_render(X3D_RasterEdge* edges, uint16 total_e, X3D_DisplayLineList* list, X3D_Color color) {
  uint16 i;
  
  for(i = 0; i < total_e; ++i) {
    X3D_Vex2D a, b;
    
    if((edges[i].flags & EDGE_INVISIBLE) == 0) {
      x3d_rasteredge_get_endpoints(edges + i, &a, &b);
      x3d_displaylinelist_add(list, a, edges[i].start.z, b, edges[i].end.z, color);
    }
  }
}

typedef struct X3D_ObjectDepth {
  X3D_DynamicObjectBase* obj;
  X3D_Vex3D pos;
  int16 depth;
  int32 dist;
} X3D_ObjectDepth;

int x3d_objectdepth_compare(X3D_ObjectDepth* a, X3D_ObjectDepth* b) {
  return b->dist - a->dist;
  
  
  if(abs(a->depth - b->depth) < 10) {
    //x3d_assert(a->dist != b->dist);
    return b->dist - a->dist;
  }
  
  return b->depth - a->depth;
}

///////////////////////////////////////////////////////////////////////////////
/// Renders any objects that are in a segment.
///////////////////////////////////////////////////////////////////////////////
void x3d_segment_render_objects(X3D_UncompressedSegment* seg, X3D_CameraObject* cam,X3D_DisplayLineList* list,
      X3D_RasterRegion* region, uint16 step) {

  uint16 i;
  
  X3D_ObjectEvent ev = {
    .type = X3D_OBJECT_EVENT_RENDER,
    .render_event = {
      .cam = cam,
      .list = list,
      .region = region
    }
  };
  
  X3D_ObjectEvent ev_frame = {
    .type = X3D_OBJECT_EVENT_FRAME
  };
  
  
  X3D_ObjectDepth depth[X3D_MAX_OBJECTS_IN_SEG];
  int16 total_d = 0;
  
  // Sort the objects from furthest to nearest depth
  for(i = 0; i < X3D_MAX_OBJECTS_IN_SEG; ++i) {
    if(seg->object_list.objects[i] != X3D_INVALID_HANDLE) {
      depth[total_d].obj = x3d_handle_deref(seg->object_list.objects[i]);
      
      x3d_object_pos(depth[total_d].obj, &depth[total_d].pos);
      x3d_camera_transform_points(cam, &depth[total_d].pos, 1, &depth[total_d].pos, NULL);
      
      depth[total_d].depth = depth[total_d].pos.z;
      depth[total_d].dist =  (int32)depth[total_d].pos.x * depth[total_d].pos.x + 
        (int32)depth[total_d].pos.y * depth[total_d].pos.y + 
        (int32)depth[total_d].pos.z * depth[total_d].pos.z;
        
      //printf("Dist: %d\n", depth[total_d].dist);
      
      ++total_d;
    }
  }
  
  
  qsort(depth, total_d, sizeof(X3D_ObjectDepth), x3d_objectdepth_compare);
  
  for(i = 0; i < total_d; ++i) {
    if(depth[i].obj->base.frame != step) {
      depth[i].obj->base.type->event_handler(depth[i].obj, ev_frame);
      depth[i].obj->base.frame = step;
    }
    
    depth[i].obj->base.type->event_handler(depth[i].obj, ev);
  }
}

uint16 x3d_segment_render_wall_portals(X3D_SegFaceID wall_id, X3D_CameraObject* cam, X3D_RasterRegion* region,
      X3D_DisplayLineList* list) {
  
  uint16 portals[32];
  uint16 total_p = x3d_wall_get_wallportals(wall_id, portals);
  
  if(total_p > 0) {
    uint16 i;
    
    //for(i = 0; i < total_p; ++i)
    //  x3d_wallportal_render(portals[i], cam, region, list);
  }
  
  return total_p;
}

void x3d_clipcontext_generate_rasteredges(X3D_ClipContext* clip, X3D_Stack* stack) {
  uint16 i;
  
  for(i = 0; i < clip->total_e; ++i) {
    uint16 a, b;
    
    a = clip->edge_pairs[i].val[0];
    b = clip->edge_pairs[i].val[1];
    
    X3D_Vex3D temp_a = clip->v3d[a], temp_b = clip->v3d[b];
    X3D_Vex2D dest_a, dest_b;
    
    if(x3d_clip_line_to_near_plane(&temp_a, &temp_b, clip->v2d + a, clip->v2d + b, &dest_a, &dest_b, x3d_rendermanager_get()->near_z) != EDGE_INVISIBLE) {
      x3d_rasteredge_generate(clip->edges + i, dest_a, dest_b, clip->parent, clip->v3d[a].z, clip->v3d[b].z, stack);
    }
    else {
      clip->edges[i].flags = EDGE_INVISIBLE;
    }
  }
}

typedef struct X3D_SegmentRenderContext {
  X3D_UncompressedSegment* seg;
  uint16 seg_id;
  X3D_UncompressedSegmentFace* faces;
  X3D_RenderManager* renderman;
  X3D_RasterRegion* parent;
  X3D_CameraObject* cam;
  X3D_ClipContext* clip;
  X3D_DisplayLineList* list;
  uint16 step;
} X3D_SegmentRenderContext;


void printnum(int16 num) {
  char buf[16];
}

void x3d_segment_render_connecting_segments(X3D_SegmentRenderContext* context) {
  uint16 i;
  
  X3D_Prism3D* prism = &context->seg->prism;
  
  // Render the connecting segments
  for(i = 0; i < x3d_prism3d_total_f(prism->base_v); ++i) {
    int16 dist = x3d_plane_dist(&context->faces[i].plane, &context->cam->pseduo_pos);
    
    if(dist > 0 || context->renderman->wireframe) {
      if(1 /*x3d_segment_render_wall_portals(x3d_segfaceid_create(id, i), cam, region, list) == 0*/) {
        if(context->faces[i].portal_seg_face != X3D_FACE_NONE) {
          void* stack_ptr = x3d_stack_save(&context->renderman->stack);
          uint16 edge_index[prism->base_v + 1];
          X3D_RasterRegion portal_region;

          uint16 face_e = x3d_prism_face_edge_indexes(prism->base_v, i, edge_index);
          
          
          X3D_RasterRegion r;
          
          context->clip->edge_index = edge_index;
          context->clip->total_edge_index = face_e;
             
          _Bool use_new = x3d_rasterregion_construct_clipped(context->clip, &portal_region);
          
          X3D_Portal portal;
          
          x3d_portal_set_fill(&portal, 31);
          
          if(use_new) {
            portal.region = &portal_region;
          }
          else if(dist <= 10) {
            portal.region = context->parent;
          }
          else {
            portal.region = NULL;
          }
          
          if(context->renderman->wireframe) {
            portal.region = context->parent;
          }
          
          if(portal.region) {
            uint16 seg_id = x3d_segfaceid_seg(context->faces[i].portal_seg_face);
            
            x3d_portal_render(&portal);
            x3d_segment_render(seg_id, context->cam, 0, portal.region, context->step);
          }
          
          
          
          x3d_stack_restore(&context->renderman->stack, stack_ptr);
        }
      }
    }
  }
}

void x3d_segment_render_textures(X3D_SegmentRenderContext* context) {
  X3D_Prism3D* prism = &context->seg->prism;
  
  uint16 i;
  // Render any textures
  for(i = 0; i < prism->base_v + 2; ++i) {
    if(context->faces[i].texture != X3D_INVALID_HANDLE) {
      X3D_Polygon3D* poly = x3d_handle_deref(context->faces[i].texture);
      X3D_Vex3D center;
      
      //printf("Handle: %d\n", face[i].texture);
      
      X3D_Polygon3D p = {
        .v = alloca(1000)
      };
      
      x3d_prism3d_get_face(prism, i, &p);
      x3d_polygon3d_center(&p, &center);
      
      X3D_Vex2D pv2d[poly->total_v];
      X3D_Vex3D pv3d[poly->total_v];
      
      uint16 d;
      for(d = 0; d < poly->total_v; ++d) {
        pv3d[d] = x3d_vex3d_add(poly->v + d, &center);
      }
      
      x3d_camera_transform_points(context->cam, pv3d, poly->total_v, pv3d, pv2d);
      
      for(d = 0; d < poly->total_v; ++d) {
        X3D_Color blue = x3d_rgb_to_color(192, 0, 192);
        
        int16 a = d;
        int16 b = (d + 1) % poly->total_v;
    
        X3D_Vex3D temp_a = pv3d[a], temp_b = pv3d[b];
        X3D_Vex2D dest_a, dest_b;
    
        if(x3d_clip_line_to_near_plane(&temp_a, &temp_b, pv2d + a, pv2d + b, &dest_a, &dest_b, x3d_rendermanager_get()->near_z) != EDGE_INVISIBLE) {
          x3d_displaylinelist_add(context->list, dest_a, pv3d[d].z, dest_b, pv3d[(d + 1) % poly->total_v].z, blue);
        }
      }
    }
  }
}

int16 depth = 0;

void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region, uint16 step  ) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  void* stack_save = x3d_stack_save(&renderman->stack);
  
  // Load the segment into the cache
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(id);
  
  X3D_DisplayLineList* list = x3d_stack_alloc(&renderman->stack, sizeof(X3D_DisplayLineList));
  list->total_l = 0;
  
  // Make sure we don't blow the stack from recursing too deep
  
  if(x3d_rendermanager_get()->wireframe) {
    if(seg->last_engine_step == step)
      return;
  }
 
  if(depth >= 16)
    return;
  
  ++depth;
  
  seg->last_engine_step = step;
  
  // Select a color based on the id of the segment because I'm too lazy to implement
  // colored segments atm
  switch(id) {
    case 0:   color = x3d_rgb_to_color(0, 0, 255); break;     // Blue
    case 1:   color = x3d_rgb_to_color(255, 0, 255); break;   // Majenta
    default:  color = 31;                                     // Red
  }  
  
  X3D_Prism3D* prism = &seg->prism;
  X3D_Vex2D* v2d = x3d_stack_alloc(&renderman->stack, sizeof(X3D_Vex2D) * prism->base_v * 2);
  X3D_Vex3D* v3d = x3d_stack_alloc(&renderman->stack, sizeof(X3D_Vex3D) * prism->base_v * 2);
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  x3d_camera_transform_points(cam, prism->v, prism->base_v * 2, v3d, v2d);
  
  X3D_UncompressedSegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  
  /// @todo It's a waste to calculate this for every edge if we don't need it,
  /// so add a bitmask to the cache to determine which faces actually need
  /// raster edges generated.
  
  uint16 total_e = seg->prism.base_v * 3;
  X3D_RasterEdge edges[total_e + 2];
  X3D_Pair edge_pair[total_e];
  
  x3d_prism_get_edge_pairs(prism->base_v, edge_pair);
  
  X3D_ClipContext clip = {
    .stack = &renderman->stack,
    .parent = region,
    .edges = edges,
    .total_e = total_e,
    .v3d = v3d,
    .v2d = v2d,
    .edge_pairs = edge_pair
  };
  
  x3d_clipcontext_generate_rasteredges(&clip, &renderman->stack);

  X3D_SegmentRenderContext context = {
    .seg = seg,
    .seg_id = id,
    .faces = face,
    .renderman = renderman,
    .parent = region,
    .cam = cam,
    .clip = &clip,
    .step = step,
    .list = list
  };
  
  x3d_rasteredge_list_render(edges, total_e, list, color);
  
  x3d_segment_render_connecting_segments(&context);
  x3d_segment_render_textures(&context);
  
  
  x3d_displaylinelist_render(list, region);
  
  // Render any objects that are in the segment
  x3d_segment_render_objects(seg, cam, list, region, step);
  
  x3d_stack_restore(&renderman->stack, stack_save);
  
  --depth;
}


void x3d_render(X3D_CameraObject* cam) {
  X3D_Color color = 31;//x3d_rgb_to_color(0, 0, 255);
  
  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);
  
  
  x3d_screen_draw_pix(x3d_enginestate_get_step() & (32 - 1), 0, 0);
  
  static int32 tick = 0;
  
  //printf("Tick: %d\n", tick++);

  line_count = 0;
  
  depth = 0;
  x3d_segment_render(cam->base.base.seg, cam, color, &x3d_rendermanager_get()->region, x3d_enginestate_get_step());
  
  printf("Line count: %d\n", line_count);
  
  int16 cx = x3d_screenmanager_get()->w / 2;
  int16 cy = x3d_screenmanager_get()->h / 2;
  
#if 0  
  x3d_screen_draw_pix(cx, cy - 1, 0xFFFF);
  x3d_screen_draw_pix(cx, cy + 1, 0xFFFF);
  x3d_screen_draw_pix(cx - 1, cy, 0xFFFF);
  x3d_screen_draw_pix(cx + 1, cy, 0xFFFF);
#endif
  
  X3D_Color red = x3d_rgb_to_color(255, 0, 0);
  X3D_Color white = x3d_rgb_to_color(255, 255, 255);
  
  //x3d_screen_draw_line_grad(0, 0, 639, 479, red, white);
}

