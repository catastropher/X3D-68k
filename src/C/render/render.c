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

int16 line_count;

extern int16 render_mode;


void x3d_rendermanager_init(X3D_InitSettings* settings) {
  // Initialize the render stack
  uint32 stack_size = 600000;
  void* render_stack_mem = malloc(stack_size);

  x3d_assert(render_stack_mem);

  X3D_RenderManager* renderman = x3d_rendermanager_get();

  // Reset segment face render callback
  renderman->segment_face_render_callback = NULL;
  
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
  
  renderman->zbuf = malloc(sizeof(int16) * screenman->w * screenman->h);
}

void x3d_rendermanager_cleanup(void) {
  free(x3d_rendermanager_get()->stack.base);
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


  //qsort(depth, total_d, sizeof(X3D_ObjectDepth), x3d_objectdepth_compare);

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
    
    int16 scale_a = clip->depth_scale[a];
    int16 scale_b = clip->depth_scale[b];
    
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

void test_clip(X3D_Polygon3D* poly, X3D_CameraObject* cam);

void x3d_segment_construct_clipped_face(X3D_SegmentRenderContext* context, uint16 face, X3D_RasterRegion** dest, X3D_RasterRegion* r, int16 dist) {
  X3D_Prism3D* prism = &context->seg->prism;            // Segment's prism
  uint16 edge_index[prism->base_v + 1];                 // Edge indexes for the raster region

  uint16 face_e = x3d_prism_face_edge_indexes(prism->base_v, face, edge_index);

  context->clip->edge_index = edge_index;
  context->clip->total_edge_index = face_e;
  
  context->clip->normal = &context->faces[face].plane.normal;

  _Bool use_new = x3d_rasterregion_construct_clipped(context->clip, r);

  if(context->faces[face].portal_seg_face != X3D_FACE_NONE && dist <= 10) {
    *dest = context->parent;
  }
  else if(use_new) {
    *dest = r;
  }
  else {
    *dest = NULL;
  }
}

void x3d_segment_render_connecting_segments(X3D_SegmentRenderContext* context) {
  uint16 i;

  X3D_Prism3D* prism = &context->seg->prism;

  // Render the connecting segments
  for(i = 0; i < x3d_prism3d_total_f(prism->base_v); ++i) {
    if(i != context->portal_face) {
      int16 dist = x3d_plane_dist(&context->faces[i].plane, &context->cam->pseduo_pos);
      
      if(dist > 0) {
          if(context->faces[i].portal_seg_face != X3D_FACE_NONE || 1) {
            void* stack_ptr = x3d_stack_save(&context->renderman->stack);
            X3D_Portal portal;
            X3D_RasterRegion r;
            
            //////
            x3d_segment_construct_clipped_face(context, i, &portal.region, &r, dist);

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


              color = colors[6];

              dot = X3D_MIN((int32)dot + 8192, 32767);
  
              if(render_mode != 3) { 
              
                color.x = (int32)color.x * dot / 32768;
                color.y = (int32)color.y * dot / 32768;
                color.z = (int32)color.z * dot / 32768;
              }

              X3D_Color c = x3d_rgb_to_color(color.x, color.y, color.z);
              
              X3D_SegmentRenderFace rface = {
                .id = x3d_segfaceid_create(context->seg_id, i),
                .color = c,
                .region = portal.region
              };
              
              if(x3d_rendermanager_get()->segment_face_render_callback)
                x3d_rendermanager_get()->segment_face_render_callback(&rface);
              

              x3d_rasterregion_fill(portal.region, rface.color);

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
    //x3d_segment_point_normal(seg, i, &normal);
    

    X3D_Vex3D d = { 0, 0, 32767 };

    fp0x16 dot = x3d_vex3d_fp0x16_dot(&d, &normal);
    
    
    dot = X3D_MIN((int32)dot + 8192, 32767);
    
    dot = X3D_MAX(dot, 0);

    if(render_mode != 3)
      depth_scale[i] = x3d_depth_scale(v3d[i].z, 10, 1500);
    //else
    //  depth_scale[i] = ((int32)x3d_depth_scale(v3d[i].z, 10, 1500) * dot) >> 15;
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
    .depth_scale = depth_scale,
    .seg = seg
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

void x3d_sphere_normal(X3D_Vex3D* center, X3D_Vex3D* v, X3D_Vex3D* dest) {
  dest->x = v->x - center->x;
  dest->y = v->y - center->y;
  dest->z = v->z - center->z;
  
  x3d_vex3d_fp0x16_normalize(dest);
}

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
  
  X3D_Vex3D norm[4];
  
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
        x3d_polygon3d_render(&p, cam, region, c, norm);
    }
    
    X3D_SWAP(top, bottom);
  }
  
}

void x3d_cube_render(X3D_Vex3D center, int16 w, X3D_CameraObject* cam, X3D_RasterRegion* region) {
  X3D_Prism3D* prism = alloca(1000);
  int16 steps = 4;
  x3d_prism3d_construct(prism, steps, w, w, (X3D_Vex3D_angle256) { ANG_45, 0, 0 }); //x3d_enginestate_get_step() * 2, x3d_enginestate_get_step() * 2, 0 });
  X3D_Polygon3D p = {
    .v = alloca(1000)
  };
  
  X3D_RasterRegion* rr = &x3d_rendermanager_get()->region;
  
  uint16 k;
  
  for(k = 0; k < 480; ++k) {
    rr->span[k].right.x = X3D_MIN(640 / 2 + k / 2, 639);
    rr->span[k].old_right_val = X3D_MIN(640 / 2 + k / 2, 639);
  }
  
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

  p.v[0] = (X3D_Vex3D) { 100, 100, 100 };
  p.v[1] = (X3D_Vex3D) { -100, -150, 100 };
  p.v[2] = (X3D_Vex3D) { 100, -100, 100 };
  
  p.total_v = 3;
  
  for(i = 0; i < steps + 2; ++i) {
    //x3d_prism3d_get_face(prism, i, &p);
    
    if(i == 2)
      x3d_polygon3d_render(&p, cam, region, 0, norm);
  }
}

void x3d_render(X3D_CameraObject* cam) {
  X3D_Color color = 31;//x3d_rgb_to_color(0, 0, 255);

  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);


  static int32 tick = 0;

  //if((x3d_enginestate_get_step() % 2) == 0)
  //  tick = (tick + 1) % 4;        
  //printf("Tick: %d\n", tick++);

  line_count = 0;
  
  depth = 0;
  x3d_segment_render(cam->base.base.seg, cam, color, &x3d_rendermanager_get()->region, x3d_enginestate_get_step(), 0xFFFF);

  X3D_Vex3D v[] = {
    { -100, 100, 0 },
    { -100, -100, 0 },
    { 100, -100, 0 },
    { 100, 100, 0 }
  };
  
  X3D_Polygon3D p = {
    .v = v,
    .total_v = 4
  };
  
  
  x3d_screen_zbuf_clear();
  
  //x3d_polygon3d_render(&p, cam, &x3d_rendermanager_get()->region, 31);
  
  int16 orbit = 100;
  int16 r = 200;
  
  angle256 angle = (x3d_enginestate_get_step() % orbit) * 256 / orbit;
  
  X3D_Vex3D pos = {
    ((int32)x3d_cos(angle) * r) >> 15,
    0,
    ((int32)x3d_sin(angle) * r) >> 15
  };
  
  //x3d_sphere_render((X3D_Vex3D) { 0, 0, 0 }, 75, 10, 31, cam, &x3d_rendermanager_get()->region);
  
  x3d_cube_render((X3D_Vex3D) { 100, 0, 100 }, 100, cam, &x3d_rendermanager_get()->region);
  //x3d_cube_render((X3D_Vex3D) { 150, 150, 150 }, 150, cam, &x3d_rendermanager_get()->region);
  
  
  //x3d_sphere_render(pos, 30, 10, x3d_rgb_to_color(0, 0, 255), cam, &x3d_rendermanager_get()->region);
  
  X3D_Vex3D v2[] = {
    { -100, 100, 100 },
    { -100, -100, 100 },
    { 100, -100, 100 },
    { 100, 100, 100 }
  };
  
  p.v = v2;
  
  //x3d_polygon3d_render(&p, cam, &x3d_rendermanager_get()->region, x3d_rgb_to_color(0, 0, 255));
  
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
  
  //x3d_screen_zbuf_visualize();

  X3D_Vex3D_angle256 ang;
  X3D_Mat3x3 mat = cam-> base.mat;
  x3d_mat3x3_transpose(&mat);
  
  //x3d_mat3x3_extract_angles(&mat, &ang);
  //x3d_log(X3D_INFO, "Real angle: %d", (angle256)cam->base.angle.y);
  
  X3D_Color red = x3d_rgb_to_color(255, 0, 0);
  X3D_Color white = x3d_rgb_to_color(255, 255, 255);

  //x3d_screen_draw_line_grad(0, 0, 639, 479, red, white);
}
