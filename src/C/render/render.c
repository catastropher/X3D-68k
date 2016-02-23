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

int16 x3d_depth_scale(int16 depth, int16 min_depth, int16 max_depth) {
  if(depth > max_depth)
    return 0;
  
  return 0x7FFF - (((int32)depth - min_depth) << 15) / (max_depth - min_depth);
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

uint16 scale_down(uint32 value, int16* error) {
  int16 v = (value >> 15) + *error;
  
  int16 lo = (v / 8) * 8;
  int16 hi = (v / 8) * 8 + 8;
  
  int16 new_v;
  
  if(v > 255)
    v = 255;
  
  if(v < 0)
    v = 0;
  
  if(abs(lo - v) < abs(hi - v)) {
    new_v = lo;
  }
  else {
    new_v = hi;
  }
  
  *error = v - new_v;
  
  return new_v;
}

X3D_Vex3D_int16 color_err;

X3D_Color x3d_color_scale(uint32 r, uint32 g, uint32 b) {
  return x3d_rgb_to_color(
    scale_down((uint32)r, &color_err.x),
    scale_down((uint32)g, &color_err.y),
    scale_down((uint32)b, &color_err.z)
  );
}

void x3d_draw_3d_line(X3D_Vex3D a, X3D_Vex3D b, X3D_CameraObject* cam, X3D_Color color) {
  X3D_Vex3D v[2] = { a, b };
  X3D_Vex2D v2d[2];
  
  x3d_camera_transform_points(cam, v, 2, v, v2d);
  
  X3D_Vex2D aa, bb;
  
  if(!(x3d_clip_line_to_near_plane(v, v + 1, v2d, v2d + 1, &aa, &bb, 10) & EDGE_INVISIBLE)) {
    x3d_screen_draw_line(aa.x, aa.y, bb.x, bb.y, color);
  }
}

void x3d_mat3x3_visualize(X3D_Mat3x3* mat, X3D_Vex3D pos, X3D_CameraObject* cam) {
  return;
  X3D_Color colors[] = {
    x3d_rgb_to_color(255, 0, 0),
    x3d_rgb_to_color(0, 255, 0),
    x3d_rgb_to_color(0, 0, 255)
  };
  
  uint16 i;
  
  for(i = 0; i < 3; ++i) {
    X3D_Vex3D p;
    x3d_mat3x3_get_column(mat, i, &p);
    
    p.x = (p.x >> 9) + pos.x;
    p.y = (p.y >> 9) + pos.y;
    p.z = (p.z >> 9) + pos.z;
    
    x3d_draw_3d_line(pos, p, cam, colors[i]);
  }
}


void x3d_rendermanager_init(X3D_InitSettings* settings) {
  // Initialize the render stack
  uint32 stack_size = 600000;
  void* render_stack_mem = malloc(stack_size);

  x3d_assert(render_stack_mem);

  X3D_RenderManager* renderman = x3d_rendermanager_get();

  x3d_stack_init(&renderman->stack, render_stack_mem, stack_size);

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
}

void x3d_rendermanager_cleanup(void) {
  free(x3d_rendermanager_get()->stack.base);
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
 return;
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

    X3D_Vex3D_int16 colors[] = {
              { 255, 0, 0 },
              { 0, 255, 0 },
              { 0, 0, 255 },
              { 255, 0, 255 },
              { 255, 255, 0 },
              { 0, 255, 255 },
              { 255, 255, 255 },
              { 255, 0, 128 },
              { 128, 64, 64 },
              { 64, 128, 64 }
            };


    if(i >= total_e / 3 * 2) {
      X3D_Vex3D_int16 c = colors[i - total_e / 3 * 2];
      color = x3d_rgb_to_color(c.x, c.y, c.z);
    }


    if((edges[i].flags & EDGE_INVISIBLE) == 0 && (edges[i].flags & EDGE_NO_DRAW) == 0) {
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

    for(i = 0; i < total_p; ++i)
      x3d_wallportal_render(portals[i], cam, region, list);
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

    uint16 res = x3d_clip_line_to_near_plane(&temp_a, &temp_b, clip->v2d + a, clip->v2d + b, &dest_a, &dest_b, x3d_rendermanager_get()->near_z);

    if(!(res & EDGE_INVISIBLE)) {
      x3d_rasteredge_generate(clip->edges + i, dest_a, dest_b, clip->parent, clip->v3d[a].z, clip->v3d[b].z, stack, clip->depth_scale[a], clip->depth_scale[b]);
      clip->edges[i].flags |= res;
    }
    else {
      clip->edges[i].flags = res;
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
  uint16 portal_face;
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

    if(i == context->portal_face)
      continue;
    
    //if(context->seg_id != 0 || i != 2)
    //  continue;

    if(dist > 0 || context->renderman->wireframe) {
      if(1) {
        if(context->faces[i].portal_seg_face != X3D_FACE_NONE || 1) {
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

          if(context->faces[i].portal_seg_face != X3D_FACE_NONE && dist <= 10) {
            portal.region = context->parent;
            printf("Close!\n");
          }
          else if(use_new) {
            portal.region = &portal_region;
          }
          else {
            portal.region = NULL;
          }

          if(portal.region && portal.region != context->parent && context->faces[i].portal_seg_face == X3D_FACE_NONE) {
            X3D_Vex3D dir;


             x3d_dynamicobject_forward_vector(context->cam, &dir);
 
             X3D_Vex3D d = { 0, 0, 32767 };
 
             fp0x16 dot = abs(x3d_vex3d_fp0x16_dot(&d, &context->faces[i].plane.normal));

            X3D_Vex3D_int16 colors[] = {
              { 255, 0, 0 },
              { 0, 255, 0 },
              { 0, 0, 255 },
              { 128, 0, 128},
              { 255, 255, 0 },
              { 0, 64, 64},
              { 255, 255, 255 },
              { 255, 0, 128 },
              { 128, 64, 64 },
              { 64, 128, 64 }
            };


            X3D_Vex3D_fp0x16 color = { 255, 0, 255 };

            int cid = 0;

            switch(context->seg_id) {
              case 0:   cid = 5; break;
              case 1:   cid = 1; break;
              case 2:   cid = 2; break;
              case 3:   cid = 3; break;
            }

            cid = context->seg_id % 10;


            color = colors[9];

             dot = X3D_MIN((int32)dot + 8192, 32767);
 
             color.x = (int32)color.x * dot / 32768;
             color.y = (int32)color.y * dot / 32768;
             color.z = (int32)color.z * dot / 32768;

            X3D_Color c = x3d_rgb_to_color(color.x, color.y, color.z);

            X3D_Polygon3D poly = {
              .v = alloca(1000)
            };

            x3d_prism3d_get_face(prism, i, &poly);

            X3D_Vex3D center;
            x3d_polygon3d_center(&poly, &center);

            x3d_camera_transform_points(context->cam, &center, 1, &center, NULL);


            //c = x3d_color_scale_by_depth(c, center.z, 10, 2000);

            x3d_rasterregion_fill(portal.region, c);

#if 0
            if(x3d_key_down(X3D_KEY_15)) {
              x3d_screen_flip();
              printf("id: %d   face: %d\n", context->seg_id, i);
              SDL_Delay(1000);
              x3d_screen_flip();
            }
#endif

            goto render_portals;
          }

          if(context->faces[i].portal_seg_face == X3D_FACE_NONE)
            goto render_portals;

          if(context->renderman->wireframe) {
            portal.region = context->parent;
          }

          if(portal.region) {
            uint16 seg_id = x3d_segfaceid_seg(context->faces[i].portal_seg_face);
            uint16 seg_face = x3d_segfaceid_face(context->faces[i].portal_seg_face);
            
            x3d_portal_render(&portal);
            x3d_segment_render(seg_id, context->cam, 0, portal.region, context->step, seg_face);
          }
          else {
render_portals:
            x3d_segment_render_wall_portals(x3d_segfaceid_create(context->seg_id, i), context->cam, context->parent, context->list);
          }

          x3d_stack_restore(&context->renderman->stack, stack_ptr);
        }
      }
    }
  }
}

void x3d_segment_render_textures(X3D_SegmentRenderContext* context) {
  X3D_Prism3D* prism = &context->seg->prism;

  // FIXME
  return;

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

void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region, uint16 step, uint16 portal_face) {
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

  if(depth >= 15)
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
  
  fp0x16 depth_scale[prism->base_v * 2];
  X3D_Vex3D cam_dir;
  
  x3d_dynamicobject_forward_vector(cam, &cam_dir);
  
  
  for(i = 0;i < prism->base_v * 2; ++i) {
    X3D_Vex3D normal;
    x3d_segment_point_normal(seg, i, &normal);
    

    X3D_Vex3D d = { 0, 0, 32767 };

    fp0x16 dot = abs(x3d_vex3d_fp0x16_dot(&d, &normal));
    
    
    dot = X3D_MIN((int32)dot + 8192, 32767);
    
#if 1
    depth_scale[i] = x3d_depth_scale(v3d[i].z, 10, 1500);
#else
    depth_scale[i] = ((int32)x3d_depth_scale(v3d[i].z, 10, 1500) * dot) >> 15;
#endif
  }

#if 1
  if(id == 5) {
    for(i = 0; i < prism->base_v * 2; ++i) {
      printf("p%d: { %d, %d }\n", i, v2d[i].x, v2d[i].y);
    }

    printf("=======================\n");
  }
#endif


  X3D_UncompressedSegmentFace* face = x3d_uncompressedsegment_get_faces(seg);

  /// @todo It's a waste to calculate this for every edge if we don't need it,
  /// so add a bitmask to the cache to determine which faces actually need
  /// raster edges generated.

  uint16 total_e = seg->prism.base_v * 3;
  X3D_RasterEdge edges[total_e + 5];
  X3D_Pair edge_pair[total_e];

  x3d_prism_get_edge_pairs(prism->base_v, edge_pair);

  X3D_ClipContext clip = {
    .stack = &renderman->stack,
    .parent = region,
    .edges = edges,
    .total_e = total_e,
    .v3d = v3d,
    .v2d = v2d,
    .edge_pairs = edge_pair,
    .depth_scale = depth_scale
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
    .list = list,
    .portal_face = portal_face
  };

  x3d_segment_render_connecting_segments(&context);

  x3d_rasteredge_list_render(edges, total_e, list, color);

  x3d_segment_render_textures(&context);


  x3d_displaylinelist_render(list, region);

  // Render any objects that are in the segment
  x3d_segment_render_objects(seg, cam, list, region, step);

  x3d_stack_restore(&renderman->stack, stack_save);

  --depth;
  
  if(id == 0 && depth == 0) {
    X3D_Vex3D a = { 0, 100, 0 };
    X3D_WallPortal* portal = x3d_wallportal_get(0);
    
    x3d_mat3x3_visualize(&portal->mat, a, cam);
  }
}


void x3d_render(X3D_CameraObject* cam) {
  X3D_Color color = 31;//x3d_rgb_to_color(0, 0, 255);

  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);


  x3d_screen_draw_pix(x3d_enginestate_get_step() & (32 - 1), 0, 0);


#if 0
  X3D_RasterRegion region;

  X3D_RasterEdge edges[10];

  uint16 edge_index[] = { 0, 3 };

  uint16 e = 2;

  X3D_Vex2D v[] = {
    { 100, 100 },
    { -50, -200 },
    { -100, -200 },
    { -100, 100 }
  };

  X3D_Vex3D v3d[10];

  uint16 i;
  for(i = 0; i < 10; ++i) {
    v3d[i].z = 100;
  }

  for(i = 0; i < e; ++i) {
    x3d_rasteredge_generate(edges + i, v[i], v[(i + 1) % 4], &x3d_rendermanager_get()->region, 10, 10, &x3d_rendermanager_get()->stack);
  }

  X3D_Pair pairs[] = {
    {{ 0, 1 }},
    {{ 1, 2 }},
    {{ 2, 3 }},
    {{ 3, 0 }},
  };

  edges[1].flags |= EDGE_INVISIBLE | EDGE_TOP_CLIPPED;
  edges[2].flags |= EDGE_INVISIBLE | EDGE_TOP_CLIPPED | EDGE_LEFT_CLIPPED;

  X3D_ClipContext context = {
    .stack = &x3d_rendermanager_get()->stack,
    .parent = &x3d_rendermanager_get()->region,
    .edges = edges,
    .edge_index = edge_index,
    .total_e = 4,
    .total_edge_index = e,
    .v3d = v3d,
    .edge_pairs = pairs,
    .v2d = v
  };

  if(x3d_rasterregion_construct_clipped(&context, &region)) {
    x3d_rasterregion_fill(&region, 31);
  }



  return;

#endif


  
  
  static int32 tick = 0;

  //if((x3d_enginestate_get_step() % 2) == 0)
  //  tick = (tick + 1) % 4;        
  //printf("Tick: %d\n", tick++);

  line_count = 0;

  depth = 0;
  x3d_segment_render(cam->base.base.seg, cam, color, &x3d_rendermanager_get()->region, x3d_enginestate_get_step(), 0xFFFF);

  
#if 0
  uint16 i;
  for(i = tick; i < x3d_screenmanager_get()->h; i+= 4) {
    x3d_screen_draw_line(0, i, x3d_screenmanager_get()->w, i, 0);
    x3d_screen_draw_line(0, i + 1, x3d_screenmanager_get()->w, i + 1, 0);
  }
#endif
  
  //printf("Line count: %d\n", line_count);

  int16 cx = x3d_screenmanager_get()->w / 2;
  int16 cy = x3d_screenmanager_get()->h / 2;

#if 1
  x3d_screen_draw_pix(cx, cy - 1, 0xFFFF);
  x3d_screen_draw_pix(cx, cy + 1, 0xFFFF);
  x3d_screen_draw_pix(cx - 1, cy, 0xFFFF);
  x3d_screen_draw_pix(cx + 1, cy, 0xFFFF);
#endif

  X3D_Vex3D_angle256 ang;
  X3D_Mat3x3 mat = cam-> base.mat;
  x3d_mat3x3_transpose(&mat);
  
  //x3d_mat3x3_extract_angles(&mat, &ang);
  //x3d_log(X3D_INFO, "Real angle: %d", (angle256)cam->base.angle.y);
  
  X3D_Color red = x3d_rgb_to_color(255, 0, 0);
  X3D_Color white = x3d_rgb_to_color(255, 255, 255);

  //x3d_screen_draw_line_grad(0, 0, 639, 479, red, white);
}
