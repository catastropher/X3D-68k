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

#include <alloca.h>

#include "X3D_common.h"
#include "X3D_prism.h"
#include "X3D_camera.h"
#include "X3D_segment.h"
#include "X3D_enginestate.h"
#include "X3D_clip.h"
#include "X3D_trig.h"
#include "X3D_collide.h"
#include "X3D_wallportal.h"


int16 x3d_scale_by_depth(int16 value, int16 depth, int16 min_depth, int16 max_depth) {  
  if(depth > max_depth)
    return 0;
  
  return value - ((int32)value * (depth - min_depth) / (max_depth - min_depth));
}

X3D_Color x3d_color_scale_by_depth(X3D_Color color, int16 depth, int16 min_depth, int16 max_depth) {
  uint8 r, g, b;
  x3d_color_to_rgb(color, &r, &g, &b);
  
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
  
  if(x3d_rasterregion_clip_line(region, &renderman->stack, &v1, &v2)) {
    X3D_Color new1 = x3d_color_scale_by_depth(color, depth1, 10, 1000);
    X3D_Color new2 = x3d_color_scale_by_depth(color, depth2, 10, 1000);
    
    x3d_screen_draw_line_grad(v1.x, v1.y, v2.x, v2.y, new1, new2);
  }
}

typedef struct X3D_ClipContext {
  X3D_Stack* stack;
  X3D_RasterRegion* parent;
  X3D_RasterEdge* edges;
  uint16* edge_index;
  uint16 total_e;
  uint16 total_edge_index;
  X3D_Vex3D* v3d;
  X3D_Vex2D* v2d;
  X3D_Pair* edge_pairs;
  _Bool really_close;
} X3D_ClipContext;

/// Under construction
_Bool x3d_construct_clipped_rasterregion(X3D_ClipContext* clip, X3D_RasterRegion* dest){

  X3D_RenderManager* renderman = x3d_rendermanager_get();
  uint16 i;
  uint16 total_vis_e = 0;   // How many edges are actually visible
  int16 near_z = 10;
  X3D_Vex2D out_v[2];
  uint16 total_out_v = 0;
  uint16 vis_e[clip->total_edge_index + 1];
  _Bool close = X3D_FALSE;
  int16 depth[2];
  
  for(i = 0; i < clip->total_edge_index; ++i) {
    X3D_Pair edge = clip->edge_pairs[clip->edge_index[i]];
    uint16 in[2] = { clip->v3d[edge.val[0]].z >= near_z, clip->v3d[edge.val[1]].z >= near_z };

    if(in[0] || in[1]) {
      vis_e[total_vis_e++] = clip->edge_index[i];
      
      if(in[0] != in[1]) {
        X3D_Vex2D v[2];
        
        x3d_rasteredge_get_endpoints(clip->edges + clip->edge_index[i], v, v + 1);
        
        // FIXME
        if(in[0]) {
          out_v[total_out_v] = v[0];
          depth[total_out_v++] = clip->edges[clip->edge_index[i]].start.z;
        }
        else {
          out_v[total_out_v] = v[1];
          depth[total_out_v++] = clip->edges[clip->edge_index[i]].end.z;
        }
          
        //out_v[total_out_v++] = v[in[0]];
        
        //clip->edge_pairs[clip->edge_index[i]].val[in[0]];
      }
    }
    else {
      close = close || clip->v3d[edge.val[0]].z >= 0 || clip->v3d[edge.val[1]].z >= 0;
    }
  }
  
  x3d_assert(total_out_v <= 2);
    
  // Create a two edge between the two points clipped by the near plane
  if(total_out_v == 2) { 
    /// FIXME please!
#if 1
    x3d_rasteredge_generate(&renderman->stack, clip->edges + clip->total_e,
      out_v[0], out_v[1], clip->parent->y_range, depth[0], depth[1]);
    
    //printf("Line: {%d,%d} - {%d,%d}, %d\n", out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y, total_vis_e + 1);
    
    //x3d_screen_draw_line(out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y, 0x7FFF);
    
    
    
    vis_e[total_vis_e++] = clip->total_e;
#endif
  }
  
  //printf("Total vis e: %d\nOut v: %d\n", total_vis_e, total_out_v);
  
  //return total_vis_e > 0 &&
  if(total_vis_e > 2) {
    if(x3d_rasterregion_construct_from_edges(dest, &renderman->stack, clip->edges, vis_e, total_vis_e) &&
      x3d_rasterregion_intersect(clip->parent, dest)) {
  
      //x3d_rasterregion_fill(dest, 0xFFFF);
  
      return X3D_TRUE;
    }
  }
  else {
    clip->really_close = close;
  }
  
  return X3D_FALSE;
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
    
    x3d_rasteredge_get_endpoints(edges + i, &a, &b);
    x3d_displaylinelist_add(list, a, edges[i].start.z, b, edges[i].end.z, color);
  }
}

void x3d_prism3d_render_wireframe(X3D_Prism3D* prism, X3D_Vex3D* translation, X3D_DisplayLineList* list, X3D_CameraObject* cam, X3D_Color color) {
  X3D_Vex3D v3d[prism->base_v * 2];
  X3D_Vex2D v2d[prism->base_v * 2];
  
  uint16 i;
  for(i = 0; i < prism->base_v * 2; ++i) {
    v3d[i].x = prism->v[i].x + translation->x;
    v3d[i].y = prism->v[i].y + translation->y;
    v3d[i].z = prism->v[i].z + translation->z;
  }
  
  x3d_camera_transform_points(cam, v3d, prism->base_v * 2, v3d, v2d);
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    uint16 a, b;
    x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
    
    X3D_Vex2D va, vb;
    
    if(x3d_clip_line_to_near_plane(v3d + a, v3d + b, v2d + a, v2d + b, &va, &vb, 10) != EDGE_INVISIBLE) {
      x3d_displaylinelist_add(list, va, v3d[a].z, vb, v3d[b].z, color);
    }
  }
}

void x3d_prism3d_render_solid(X3D_Prism3D* prism, X3D_Vex3D* translation, X3D_DisplayLineList* list, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region) {
  X3D_Vex3D v3d[prism->base_v * 2];
  X3D_Vex2D v2d[prism->base_v * 2];
  
  uint16 i;
  for(i = 0; i < prism->base_v * 2; ++i) {
    v3d[i].x = prism->v[i].x + translation->x;
    v3d[i].y = prism->v[i].y + translation->y;
    v3d[i].z = prism->v[i].z + translation->z;
  }
  
  x3d_camera_transform_points(cam, v3d, prism->base_v * 2, v3d, v2d);
  
  X3D_RasterEdge edge[prism->base_v * 3];
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    uint16 a, b;
    x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
    
    X3D_Vex2D va, vb;
    
    if(x3d_clip_line_to_near_plane(v3d + a, v3d + b, v2d + a, v2d + b, &va, &vb, 10) != EDGE_INVISIBLE) {
      //x3d_displaylinelist_add(list, va, v3d[a].z, vb, v3d[b].z, color);
      x3d_rasteredge_generate(&renderman->stack, edge + i, va, vb, region->y_range, v3d[a].z, v3d[b].z);
    }
    else {
      edge[i].flags |= EDGE_INVISIBLE;
    }
  }
  
  uint16 edge_list[prism->base_v * 3];
  
  for(i = 0; i < prism->base_v * 3; ++i)
    edge_list[i] = i;
  
  X3D_RasterRegion new_region;
#if 1
  if(x3d_rasterregion_construct_from_edges(&new_region, &renderman->stack, edge, edge_list, prism->base_v * 3)) {
    if(x3d_rasterregion_intersect(region, &new_region)) {
      x3d_rasterregion_fill(&new_region, 0);
    }
  }
#endif
  
  _Bool render_edge[prism->base_v * 3];
  
  for(i = 0; i < prism->base_v * 3; ++i)
    render_edge[i] = X3D_FALSE;
  
  X3D_Polygon3D poly = {
    .v = alloca(sizeof(X3D_Vex3D) * prism->base_v)
  };
  
  X3D_Plane plane;
  
  X3D_Vex3D cam_pos = cam->pseduo_pos;
  //x3d_object_pos(cam, &cam_pos);
  
  for(i = 0; i < 2; ++i) {//prism->base_v + 2; ++i) {
    x3d_prism3d_get_face(prism, i, &poly);
    
    
    poly.v[0].x += translation->x;
    poly.v[0].y += translation->y;
    poly.v[0].z += translation->z;
    
    poly.v[1].x += translation->x;
    poly.v[1].y += translation->y;
    poly.v[1].z += translation->z;
    
    poly.v[2].x += translation->x;
    poly.v[2].y += translation->y;
    poly.v[2].z += translation->z;
    
    x3d_plane_construct(&plane, poly.v, poly.v + 1, poly.v + 2);
    
    int16 dist = x3d_plane_dist(&plane, &cam_pos);
    
    if(dist <= 0) {
      int16 d;
      uint16 e[prism->base_v];
      
      uint16 total_e = x3d_prism_face_edge_indexes(prism->base_v, i, e);
      
      for(d = 0; d < total_e; ++d) {
        render_edge[e[d]] = X3D_TRUE;
      }
    }
  }
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    X3D_RasterEdge* e = edge + i;
    
    if(render_edge[i]) {
      uint16 a, b;
      x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
      
      x3d_draw_clipped_line(e->start.x, e->start.y, e->end.x, e->end.y, v3d[a].z, v3d[b].z, color, region);
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
  
  printf("\n");
  
  qsort(depth, total_d, sizeof(X3D_ObjectDepth), x3d_objectdepth_compare);
  
  x3d_displaylinelist_render(list, region);
  
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
    
    if(x3d_clip_line_to_near_plane(&temp_a, &temp_b, clip->v2d + a, clip->v2d + b, &dest_a, &dest_b, 10) != EDGE_INVISIBLE) {
      x3d_rasteredge_generate(stack, clip->edges + i, dest_a, dest_b, clip->parent->y_range, clip->v3d[a].z, clip->v3d[b].z);
    }
  }
}

void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region, uint16 step) {
  // Load the segment into the cache
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(id);
  
  X3D_DisplayLineList* list = alloca(sizeof(X3D_DisplayLineList));
  list->total_l = 0;
  
  // Make sure we don't blow the stack from recursing too deep
  static int16 depth = 0;
  
  if(depth >= 16)
    return;
  
  ++depth;
  
  
  // Select a color based on the id of the segment because I'm too lazy to implement
  // colored segments atm
  switch(id) {
    case 0:   color = x3d_rgb_to_color(0, 0, 255); break;     // Blue
    case 1:   color = x3d_rgb_to_color(255, 0, 255); break;   // Majenta
    default:  color = 31;                                     // Red
  }  
  
  X3D_Prism3D* prism = &seg->prism;
  X3D_Vex2D v2d[prism->base_v * 2];
  X3D_Vex3D v3d[prism->base_v * 2];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  x3d_camera_transform_points(cam, prism->v, prism->base_v * 2, v3d, v2d);
  
  X3D_UncompressedSegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  void* stack_save = x3d_stack_save(&renderman->stack);
  
  /// @todo It's a waste to calculate this for every edge if we don't need it,
  /// so add a bitmask to the cache to determine which faces actually need
  /// raster edges generated.
  
  uint16 total_e = seg->prism.base_v * 3;
  X3D_RasterEdge edges[total_e + 1];
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

  // Render the connecting segments
  for(i = 0; i < x3d_prism3d_total_f(seg->prism.base_v); ++i) {
    // The distance calculation shouldn't be necessary because we should always
    // be inside a segment...
    int16 dist = x3d_plane_dist(&face[i].plane, &cam->pseduo_pos);
    
    if(dist > 0) {
      if(x3d_segment_render_wall_portals(x3d_segfaceid_create(id, i), cam, region, list) == 0) {
        if(face[i].portal_seg_face != X3D_FACE_NONE) {
          void* stack_ptr = x3d_stack_save(&renderman->stack);
          uint16 edge_index[prism->base_v + 1];
          X3D_RasterRegion new_region;

          
          uint16 face_e = x3d_prism_face_edge_indexes(prism->base_v, i, edge_index);
          
          
          X3D_RasterRegion r;
          X3D_RasterRegion* portal;
          
          clip.edge_index = edge_index;
          clip.total_edge_index = face_e;
             
          _Bool use_new = x3d_construct_clipped_rasterregion(&clip, &r);
          
          if(use_new) {
            portal = &r;
          }
          else if(clip.really_close) {
            //printf("Really close!\n");
            portal = region;
          }
          else {
            portal = NULL;
          }
          
          if(portal) {
            uint16 seg_id = x3d_segfaceid_seg(face[i].portal_seg_face);
            
            x3d_segment_render(seg_id, cam, color * 8, &r, step);
          }
          
          
          
          x3d_stack_restore(&renderman->stack, stack_ptr);
        }
      }
    }
  }
  
  x3d_rasteredge_list_render(edges, total_e, list, color);
  
  // Render any objects that are in the segment
  x3d_segment_render_objects(seg, cam, list, region, step);
  
  x3d_stack_restore(&renderman->stack, stack_save);
  
  --depth;
}


void x3d_render(X3D_CameraObject* cam) {
  X3D_Color color = 31;//x3d_rgb_to_color(0, 0, 255);
  
  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);
  
  
  static int32 tick = 0;
  
  //printf("Tick: %d\n", tick++);
  
  x3d_segment_render(cam->base.base.seg, cam, color, &x3d_rendermanager_get()->region, x3d_enginestate_get_step());
  
  int16 cx = x3d_screenmanager_get()->w / 2;
  int16 cy = x3d_screenmanager_get()->h / 2;
  
  
  x3d_screen_draw_pix(cx, cy - 1, 0xFFFF);
  x3d_screen_draw_pix(cx, cy + 1, 0xFFFF);
  x3d_screen_draw_pix(cx - 1, cy, 0xFFFF);
  x3d_screen_draw_pix(cx + 1, cy, 0xFFFF);
  
  X3D_Color red = x3d_rgb_to_color(255, 0, 0);
  X3D_Color white = x3d_rgb_to_color(255, 255, 255);
  
  //x3d_screen_draw_line_grad(0, 0, 639, 479, red, white);
}

