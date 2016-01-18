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
static void x3d_draw_clipped_line(int16 x1, int16 y1, int16 x2, int16 y2, X3D_Color color, X3D_RasterRegion* region) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  X3D_Vex2D v1 = { x1, y1 };
  X3D_Vex2D v2 = { x2, y2 };
  
  if(x3d_rasterregion_clip_line(region, &renderman->stack, &v1, &v2)) {
    x3d_screen_draw_line(v1.x, v1.y, v2.x, v2.y, color);
  }
}




_Bool x3d_construct_clipped_rasterregion(X3D_RasterRegion* dest, X3D_RasterRegion* parent,
      X3D_RasterEdge* edges, uint16* edge_index, uint16 total_e, uint16 total_edge_index,
      X3D_Vex3D* v3d, X3D_Vex2D* v2d, X3D_Pair* edge_pairs) {

  X3D_RenderManager* renderman = x3d_rendermanager_get();
  uint16 i;
  uint16 total_vis_e = 0;   // How many edges are actually visible
  int16 near_z = 10;
  uint16 out_v[2];
  uint16 total_out_v = 0;
  uint16 vis_e[total_edge_index + 1];
  
  for(i = 0; i < total_edge_index; ++i) {
    X3D_Pair edge = edge_pairs[edge_index[i]];
    uint16 in[2] = { v3d[edge.val[0]].z >= near_z, v3d[edge.val[1]].z >= near_z };

    if(in[0] || in[1]) {
      vis_e[total_vis_e++] = i;
      
      if(in[0] != in[1])
        out_v[total_out_v++] = edge_pairs[i].val[in[0]];
    }
  }
  
  x3d_assert(total_out_v <= 2);
    
  // Create a two edge between the two points clipped by the near plane
  if(total_out_v == 2) { 
    x3d_rasteredge_generate(&renderman->stack, edges + total_edge_index,
      v2d[out_v[0]], v2d[out_v[1]], parent->y_range);
    
    vis_e[total_vis_e++] = total_edge_index;
  }
  
  return total_vis_e > 0 &&
    x3d_rasterregion_construct_from_edges(dest, &renderman->stack, edges, vis_e, total_vis_e) &&
    x3d_rasterregion_intersect(parent, dest);
}

///////////////////////////////////////////////////////////////////////////////
/// Renders a wall portal and anything that can be seen through the portal
///   from the perspective of the camera looking through it.
///
/// @param wall_portal_id - ID of the wall portal
/// @param cam            - camera object that is looking though the portal
/// @param region         - raster region that describes the clipping region
///
/// @return The number of portals attached to the given face.
///////////////////////////////////////////////////////////////////////////////
void x3d_wallportal_render(uint16 wall_portal_id, X3D_CameraObject* cam, X3D_RasterRegion* region) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  // Save the stack pointer so we can free any allocations made later
  void* stack_ptr = x3d_stack_save(&renderman->stack);
  
  // The wall portal to be rendered (using the temporary global implementation)
  X3D_WallPortal* portal = x3d_wallportal_get(wall_portal_id);
  
  // Outline of the portal projected onto the screen
  X3D_Vex2D v2d[portal->portal_poly.total_v];
  
  // 3D points that make up the outline and have been rotated relative to the
  // camera, but haven't been projected yet
  X3D_Vex3D v3d[portal->portal_poly.total_v];
  
  // Transform the points so they are relative to the camera
  x3d_camera_transform_points(cam, portal->portal_poly.v, portal->portal_poly.total_v,
    v3d, v2d);
  
  uint16 i;
  
  // Check for portals that are partially behind the camera (clipping against
  // the near plane isn't impelented yet)
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    // Can't handle portals that are partially behind the camera
    if(v3d[i].z < 10)
      return;
  }
  
  X3D_RasterEdge edges[portal->portal_poly.total_v];
  
  // Construct the portal's raster region, which has to be clipped against the
  // parent raster region
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    uint16 a, b;
    
    a = i;
    b = i + 1 < portal->portal_poly.total_v ? i + 1 : 0;
    
    x3d_rasteredge_generate(&renderman->stack, edges + i, v2d[a], v2d[b], region->y_range);
  }
  
  X3D_RasterRegion clipped_region;
  uint16 edge_index[portal->portal_poly.total_v];
  
  for(i = 0; i < portal->portal_poly.total_v; ++i)
    edge_index[i] = i;
  
  // The new camera that has been adjusted to rasterize things on the other
  // side of the portal properly.
  X3D_CameraObject new_cam = *cam;
  
  // Rending through a portal is a bit complicated. We have two portals that
  // are in different parts of the world and that are oriented in different
  // directions. When a camera looks through portal A, they are actually
  // looking into the room with portal B as follows:
  //
  //  __________         _________
  //  |        |         |       |
  //  |        |         |   B   |
  //  O<-- A   |         |   ^   |
  //  |________|         |___|___|
  //
  // However, we have to make it look like this when rendering the portal:
  //
  //          __________
  //  ________|        |
  // |        |        |
  // |    B<--O<-- A   |
  // |________|________|
  //
  //
  if(portal->portal_id != 0xFFFF && x3d_rasterregion_construct_from_edges(&clipped_region, &renderman->stack, edges, edge_index, portal->portal_poly.total_v)) {
    // Portal on the other side of the wall
    X3D_WallPortal* other_side = x3d_wallportal_get(portal->portal_id);
    
    X3D_Mat3x3 other_side_transpose = other_side->mat;
    x3d_mat3x3_transpose(&other_side_transpose);
    
    X3D_Mat3x3 temp;
    x3d_mat3x3_mul(&temp, &portal->mat, &other_side_transpose);
    
    X3D_Mat3x3 temp2;
    X3D_Mat3x3 rot;
    
    // By this point, portal B has been flipped by 180 degrees, so flip
    // it back
    X3D_Vex3D_angle256 angle = { 0, ANG_180, 0 };
    x3d_mat3x3_construct(&rot, &angle);
    
    x3d_mat3x3_mul(&temp2, &rot, &temp);
    x3d_mat3x3_mul(&new_cam.base.mat, &temp2, &cam->base.mat);
    
    
    X3D_Vex3D cam_pos;
    x3d_object_pos(cam, &cam_pos);
    
    X3D_Vex3D center, c = x3d_vex3d_sub(&other_side->center, &cam_pos);

    c.x += cam->shift.x;
    c.y += cam->shift.y;
    c.z += cam->shift.z;
    
    x3d_vex3d_int16_rotate(&center, &c, &new_cam.base.mat);
    
    X3D_Vex3D pcenter, pc = x3d_vex3d_sub(&portal->center, &cam_pos);
    
    pc.x += cam->shift.x;
    pc.y += cam->shift.y;
    pc.z += cam->shift.z;
    
    x3d_vex3d_int16_rotate(&pcenter, &pc, &cam->base.mat);
    
    new_cam.shift.x = pcenter.x - center.x + cam->shift.x;
    new_cam.shift.y = pcenter.y - center.y + cam->shift.y;
    new_cam.shift.z = pcenter.z - center.z + cam->shift.z;
    
    new_cam.pseduo_pos = other_side->center;
    
    if(x3d_rasterregion_intersect(region, &clipped_region)) {
      uint16 seg_id = x3d_segfaceid_seg(other_side->face);
      
      x3d_segment_render(seg_id, &new_cam, 31, &clipped_region, x3d_enginestate_get_step());
    }
  }
  
  // Draw the portal outline
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    uint16 a, b;
    X3D_Vex2D va, vb;
    
    a = i;
    b = i + 1 < portal->portal_poly.total_v ? i + 1 : 0;
    
    if(x3d_clip_line_to_near_plane(v3d + a, v3d + b, v2d + a, v2d + b, &va, &vb, 10) != EDGE_INVISIBLE) {
      x3d_draw_clipped_line(va.x, va.y, vb.x, vb.y, portal->color, region);
    }
  }
  
  x3d_stack_restore(&renderman->stack, stack_ptr);
}


void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region, uint16 step) {
  // Load the segment into the cache
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(id);
  
  static int16 depth = 0;
  
  if(depth >= 16)
    return;
  
  ++depth;
  
  if(id == 0) {
    color = x3d_rgb_to_color(0, 0, 255);
  }
  else if (id == 1) {
    color = x3d_rgb_to_color(255, 0, 255);
  }
  else {
    color = 31;
  }
  
  //printf("Step: %d, seg: %d\n", step, seg->last_engine_step);
  
  //if(seg->last_engine_step != step)
  //  return;
  
  seg->last_engine_step = step;
  
  X3D_Prism3D* prism = &seg->prism;
  X3D_Vex2D v[prism->base_v * 2];
  X3D_Vex3D v3d[prism->base_v * 2];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  x3d_camera_transform_points(cam, prism->v, prism->base_v * 2, v3d, v);
  
  //x3d_rasterregion_fill(region, 0x7FFF / (depth));
  
  //x3d_prism3d_render(&seg->prism, cam, color);
  
  
  X3D_UncompressedSegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  void* stack_save = x3d_stack_save(&renderman->stack);
  
  /// @todo It's a waste to calculate this for every edge if we don't need it,
  /// so add a bitmask to the cache to determine which faces actually need
  /// raster edges generated.
  
  uint16 total_e = seg->prism.base_v * 3;
  X3D_RasterEdge edges[total_e];
  
  for(i = 0; i < total_e; ++i) {
    uint16 a, b;
    
    x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
    x3d_rasteredge_generate(&renderman->stack, edges + i, v[a], v[b], region->y_range);
  }

  // Render the connecting segments
  for(i = 0; i < x3d_prism3d_total_f(seg->prism.base_v); ++i) {
    uint16 portals[32];
    
    uint16 total_p = x3d_wall_get_wallportals(x3d_segfaceid_create(id, i), portals);
    
    //if(id == 6 && total_p == 0) continue;
    
    if(face[i].portal_seg_face != X3D_FACE_NONE || total_p != 0) {// || (id == 0 && i == 3)) {
      // Only render the segment if we're not on the opposite side of the wall
      // with the portal
      int16 dist = x3d_plane_dist(&face[i].plane, &cam->pseduo_pos);
      
      //printf("Dist: %d\n", dist);
      
      if(dist > 0) {
        if(total_p > 0) {
          uint16 i;
          
          for(i = 0; i < total_p; ++i)
            x3d_wallportal_render(portals[i], cam, region);
        }
        else {
          void* stack_ptr = x3d_stack_save(&renderman->stack);
          uint16 edge_index[prism->base_v];
          X3D_RasterRegion new_region;

          
          uint16 face_e = x3d_prism_face_edge_indexes(prism->base_v, i, edge_index);
          
          if(x3d_rasterregion_construct_from_edges(&new_region, &renderman->stack, edges, edge_index, face_e)) {
            if(x3d_rasterregion_intersect(region, &new_region)) {
              uint16 seg_id = x3d_segfaceid_seg(face[i].portal_seg_face);
              
  #if 0
              if(id == 0)
                x3d_rasterregion_fill(&new_region, 0xFFFF);
              else
                x3d_rasterregion_fill(&new_region, 31);
  #endif
              
              x3d_segment_render(seg_id, cam, color * 8, &new_region, step);
            }
          }
          
          
          
          x3d_stack_restore(&renderman->stack, stack_ptr);
        }
      }
    }
  }
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    uint16 a, b;
    X3D_Vex2D va, vb;
    x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
    
    if(x3d_clip_line_to_near_plane(v3d + a, v3d + b, v + a, v + b, &va, &vb, 10) != EDGE_INVISIBLE) {
      x3d_draw_clipped_line(va.x, va.y, vb.x, vb.y, color, region);
    }
  }
  
  x3d_stack_restore(&renderman->stack, stack_save);
  
  --depth;
}


void x3d_render(X3D_CameraObject* cam) {
  X3D_Color color = 31;//x3d_rgb_to_color(0, 0, 255);
  
  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);
  
  
  
  x3d_segment_render(0, cam, color, &x3d_rendermanager_get()->region, x3d_enginestate_get_step());
  
  int16 cx = x3d_screenmanager_get()->w / 2;
  int16 cy = x3d_screenmanager_get()->h / 2;
  
  
  x3d_screen_draw_pix(cx, cy - 1, 0xFFFF);
  x3d_screen_draw_pix(cx, cy + 1, 0xFFFF);
  x3d_screen_draw_pix(cx - 1, cy, 0xFFFF);
  x3d_screen_draw_pix(cx + 1, cy, 0xFFFF);
  
  
}
