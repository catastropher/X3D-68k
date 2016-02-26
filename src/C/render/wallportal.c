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
#include "X3D_segment.h"
#include "X3D_enginestate.h"
#include "X3D_wallportal.h"
#include "X3D_trig.h"
#include "X3D_clip.h"
#include "X3D_portal.h"

#define X3D_MAX_WALL_PORTALS 32

// These are part of the temporary wall portal implementation
static X3D_WallPortal wall_portals[32];     // Table of all wall portals

void x3d_wallportals_init(void) {
  uint16 i;

  for(i = 0; i < X3D_MAX_WALL_PORTALS; ++i) {
    wall_portals[i].face = X3D_FACE_NONE;
    wall_portals[i].portal_poly.v = wall_portals[i].v;
  }
}

void x3d_wallportal_construct(uint16 wall_portal, X3D_SegFaceID face, X3D_Vex3D c, uint16 portal_id, X3D_Polygon2D* poly, X3D_Color color) {
  X3D_WallPortal* portal = wall_portals + wall_portal;

  portal->face = face;
  //portal->center = center;
  portal->portal_id = portal_id;
  portal->color = color;

  uint16 seg_id = x3d_segfaceid_seg(face);
  uint16 face_id = x3d_segfaceid_face(face);

  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(seg_id);

  x3d_assert(seg_id == seg->base.id);

  /// @todo Add support for BASE_A and BASE_B face portals
  //x3d_assert(face_id >= 2);

  X3D_Polygon3D p = {
    .v = alloca(sizeof(X3D_Vex3D_int16) * seg->prism.base_v)
  };
  
  x3d_prism3d_get_face(&seg->prism, face_id, &p);
  
  X3D_Vex3D center;
  x3d_polygon3d_center(&p, &center);
  
  // Project the 2D polygon onto the wall
  x3d_polygon2d_to_polygon3d(
    poly,
    &portal->portal_poly,
    &x3d_uncompressedsegment_get_faces(seg)[face_id].plane,
    &center,
    &center,
    &portal->mat
  );

  // Move the portal polygon to the center of the wall
  X3D_Polygon3D wall = {
    .v = alloca(1000)
  };

  x3d_prism3d_get_face(&seg->prism, face_id, &wall);

  uint16 i;

  if((c.x | c.y | c.z) == 0) {
    // Calculate the center of the wall
    X3D_Vex3D_int16 center = { 0, 0, 0 };

    for(i = 0; i < wall.total_v; ++i) {
      center.x += wall.v[i].x;
      center.y += wall.v[i].y;
      center.z += wall.v[i].z;
    }

    center.x /= wall.total_v;
    center.y /= wall.total_v;
    center.z /= wall.total_v;

    portal->center = (X3D_Vex3D) { center.x, center.y, center.z };
  }
  else {
    portal->center = c;
  }

  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    portal->portal_poly.v[i].x += portal->center.x;
    portal->portal_poly.v[i].y += portal->center.y;
    portal->portal_poly.v[i].z += portal->center.z;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Adds a new wall portal to a wall.
///
/// @param face       - wall to add portal to
/// @param center     - center of 3D portal on wall in world coordinates, or
///                     (0, 0, 0) to automatically center
/// @param portal_id  - wall portal ID this portal is connected to
/// @param poly       - 2D portal shape
///
/// @return The ID of the portal.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_wallportal_add(X3D_SegFaceID face, X3D_Vex3D c, uint16 portal_id, X3D_Polygon2D* poly, X3D_Color color) {
  static X3D_WallPortal* wall_portal_ptr = wall_portals;     // Location of next free wall portal

  X3D_WallPortal* portal = wall_portal_ptr++;
  portal->id = portal - wall_portals;

  x3d_wallportal_construct(portal->id, face, c, portal_id, poly, color);

  return portal->id;
}


///////////////////////////////////////////////////////////////////////////////
/// Gets a list of wall portals attached to a face.
///
/// @param face - face ID
/// @param dest - where to write portal ID's
///
/// @return The number of portals attached to the given face.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_wall_get_wallportals(X3D_SegFaceID face, uint16* dest) {
  uint16 total = 0;
  uint16 i;

  for(i = 0; i < X3D_MAX_WALL_PORTALS; ++i) {
    if(wall_portals[i].face == face) {
      dest[total++] = i;
    }
  }

  return total;
}

void x3d_wallportal_update(uint16 id) {
  X3D_WallPortal* from = wall_portals + id;
  X3D_WallPortal* to = wall_portals + from->portal_id;

  // Calculate the transformation matrix that partly calculates the view
  // transformation from portal_from to portal_to
  X3D_Mat3x3 other_side_transpose = to->mat;
  x3d_mat3x3_transpose(&other_side_transpose);

  X3D_Mat3x3 temp;
  x3d_mat3x3_mul(&temp, &from->mat, &other_side_transpose);

  X3D_Mat3x3 rot;

  // By this point, portal B has been flipped by 180 degrees, so flip
  // it back
  X3D_Vex3D_angle256 angle = { 0, ANG_180, 0 };
  x3d_mat3x3_construct(&rot, &angle);

  x3d_mat3x3_mul(&from->transform, &rot, &temp);
}

///////////////////////////////////////////////////////////////////////////////
/// Connects a wall portal to another wall portal.
///
/// @param portal_from  - portal to create connection from
/// @param portal_to    - portal that portal_from becomes connected to
///
/// @return Nothing.
/// @note This overwrites what portal_from was previously connected to.
///////////////////////////////////////////////////////////////////////////////
void x3d_wallportal_connect(uint16 portal_from, uint16 portal_to) {
  X3D_WallPortal* from = wall_portals + portal_from;

  from->portal_id = portal_to;

  x3d_wallportal_update(portal_from);
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the address of a wallportal.
///
/// @param portal_id  - ID of the portal
///
/// @return Address of the wallportal.
///////////////////////////////////////////////////////////////////////////////
X3D_WallPortal* x3d_wallportal_get(uint16 portal_id) {
  return wall_portals + portal_id;
}

#include <stdarg.h>

void x3d_mat3x3_mul_chain(X3D_Mat3x3* dest, const char* format, ...) {
  va_list list;
  va_start(list, format);
  
  X3D_Mat3x3 res;
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };
  x3d_mat3x3_construct(&res, &angle);
  
  while(*format) {
    X3D_Mat3x3* mat = va_arg(list, X3D_Mat3x3*);
    X3D_Mat3x3 transpose;
    
    if(*format == 't') {
      transpose = *mat;
      x3d_mat3x3_transpose(&transpose);
      mat = &transpose;
    }
    
    X3D_Mat3x3 temp;
    x3d_mat3x3_mul(&temp, &res, mat);
    res = temp;
    
    ++format;
  }
  
  *dest = res;
}

void x3d_wallportal_transform_point(X3D_WallPortal* portal, X3D_Vex3D* v, X3D_Vex3D* dest) {
  X3D_Vex3D diff = { v->x - portal->center.x, v->y - portal->center.y, v->z - portal->center.z };
  X3D_Mat3x3 copy = portal->transform;
  
  x3d_mat3x3_transpose(&copy);
  
  x3d_vex3d_int16_rotate(dest, &diff, &copy);
  
  X3D_WallPortal* other_side = x3d_wallportal_get(portal->portal_id);
  
  dest->x += other_side->center.x;
  dest->y += other_side->center.y;
  dest->z += other_side->center.z;
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
#if 1
void x3d_wallportal_render(uint16 wall_portal_id, X3D_CameraObject* cam, X3D_RasterRegion* region, X3D_DisplayLineList* list) {
  return;
  #if 1
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


  X3D_RasterEdge edges[portal->portal_poly.total_v + 1];

  X3D_Pair edge_pair[portal->portal_poly.total_v];

  // Construct the portal's raster region, which has to be clipped against the
  // parent raster region
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    uint16 a, b;

    a = i;
    b = i + 1 < portal->portal_poly.total_v ? i + 1 : 0;

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
  uint16 edge_index[portal->portal_poly.total_v + 1];

  for(i = 0; i < portal->portal_poly.total_v; ++i)
    edge_index[i] = i;

  X3D_ClipContext clip = {
    .stack = &renderman->stack,
    .parent = region,
    .edges = edges,
    .total_e = portal->portal_poly.total_v,
    .v3d = v3d,
    .v2d = v2d,
    .edge_pairs = edge_pair,
    .edge_index = edge_index,
    .total_edge_index = portal->portal_poly.total_v
  };

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
  
  
  
  X3D_WallPortal* other_side = x3d_wallportal_get(portal->portal_id);
  
  X3D_Mat3x3 new_mat;
  
  X3D_Mat3x3 m4;
  X3D_Vex3D_angle256 ang = { 0, 0, 0 };
  m4 = portal->mat;
  
  {
    X3D_Vex3D z;
    x3d_mat3x3_get_column(&m4, 2, &z);
    
    z = x3d_vex3d_neg(&z);
    x3d_mat3x3_set_column(&m4, 2, &z);
    
    X3D_Vex3D x;
    x3d_mat3x3_get_column(&m4, 0, &x);
    
    x = x3d_vex3d_neg(&x);
    x3d_mat3x3_set_column(&m4, 0, &x);
  }
  
  
  x3d_mat3x3_mul_chain(&new_mat, "mt", &other_side->mat, &m4);
    
  
  X3D_Vex3D z;
  x3d_mat3x3_get_column(&new_mat, 2, &z);
  
  //z = x3d_vex3d_neg(&z);
  x3d_mat3x3_set_column(&new_mat, 2, &z);
  
  #if 0
      X3D_Vex3D x;
      x3d_mat3x3_get_column(&new_mat, 0, &x);
      x = x3d_vex3d_neg(&x);
      x3d_mat3x3_set_column(&new_mat, 0, &x);
      
      
      
      X3D_Vex3D y;
      x3d_mat3x3_get_column(&new_mat, 1, &y);
      
      y = x3d_vex3d_neg(&y);
      x3d_mat3x3_set_column(&new_mat, 1, &y);
#endif  

  if(portal->portal_id != 0xFFFF) {
    _Bool use_new = x3d_rasterregion_construct_clipped(&clip, &clipped_region);

    
    uint16 seg = x3d_segfaceid_seg(portal->face);
    uint16 face = x3d_segfaceid_face(portal->face);
    
    
    
    X3D_Plane* plane = &x3d_uncompressedsegment_get_faces(x3d_segmentmanager_load(seg))[face].plane;
    
    X3D_Vex3D cam_pos;
    x3d_object_pos(cam, &cam_pos);
    
    int16 dist = x3d_plane_dist(plane, &cam_pos);
    
    X3D_Portal port;

    x3d_portal_set_fill(&port, 31);

    if(dist <= 10) {
      port.region = region;
      printf("Close!\n");
    }
    else if(use_new) {
      port.region = &clipped_region;
    }
    else {
      port.region = NULL;
    }
    
    if(port.region != NULL) {


      // Portal that 'portal' is connected to
      X3D_WallPortal* other_side = x3d_wallportal_get(portal->portal_id);

      // Calculate the new view matrix
      x3d_mat3x3_mul(&new_cam.base.mat, &new_mat, &cam->base.mat);

      // Calculate the geometry shift for everything on the other side of the portal
      //x3d_camera_calculate_shift(&new_cam, cam, &portal->center, &other_side->center);

      uint16 seg_id = x3d_segfaceid_seg(other_side->face);
      uint16 seg_face = x3d_segfaceid_face(other_side->face);
      
      X3D_Color c = x3d_rgb_to_color(255, 69, 0);
      
      portal->transform = new_mat;
      
      X3D_Vex3D cam_pos;
      x3d_object_pos(cam, &cam_pos);
      
      
      X3D_Vex3D new_diff;
      x3d_wallportal_transform_point(portal, &cam_pos, &new_diff);
      
      new_cam.base.base.pos.x = (int32)(new_diff.x) << 8;
      new_cam.base.base.pos.y = (int32)(new_diff.y) << 8;
      new_cam.base.base.pos.z = (int32)(new_diff.z) << 8;
      
      
      new_cam.pseduo_pos = new_diff;
      
      //x3d_rasterregion_fill(&clipped_region, c);

      x3d_segment_render(seg_id, &new_cam, 31, port.region, x3d_enginestate_get_step(), seg_face);
    }
  }

#if 1
  // Draw the portal outline
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    uint16 a, b;
    X3D_Vex2D va, vb;

    a = i;
    b = i + 1 < portal->portal_poly.total_v ? i + 1 : 0;

    if((x3d_clip_line_to_near_plane(v3d + a, v3d + b, v2d + a, v2d + b, &va, &vb, 10) & EDGE_INVISIBLE) == 0) {
      x3d_draw_clipped_line(va.x, va.y, vb.x, vb.y, v3d[edge_pair[i].val[0]].z, v3d[edge_pair[i].val[1]].z, 0, region);

      //x3d_displaylinelist_add(list, va, v3d[edge_pair[i].val[0]].z, vb, v3d[edge_pair[i].val[1]].z, portal->color);
    }
  }
#endif

  x3d_stack_restore(&renderman->stack, stack_ptr);
#endif
}

#endif
