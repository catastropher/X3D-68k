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
#include "X3D_segment.h"
#include "X3D_enginestate.h"
#include "X3D_wallportal.h"
#include "X3D_trig.h"

#define X3D_MAX_WALL_PORTALS 32

// These are part of the temporary wall portal implementation
static X3D_WallPortal wall_portals[32];     // Table of all wall portals

void x3d_wallportals_init(void) {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_WALL_PORTALS; ++i) {
    wall_portals[i].face = X3D_FACE_NONE;
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
  
  uint16 top_left_v = face_id - 2;
  uint16 bottom_right_v = ((top_left_v + 1) % seg->prism.base_v) + seg->prism.base_v;
  
  // Project the 2D polygon onto the wall
  x3d_polygon2d_to_polygon3d(
    poly,
    &portal->portal_poly,
    &x3d_uncompressedsegment_get_faces(seg)[face_id].plane,
    seg->prism.v + top_left_v,
    seg->prism.v + bottom_right_v,
    &portal->mat
  );
  
  // Move the portal polygon to the center of the wall
  X3D_Polygon3D* wall = alloca(1000);//x3d_polygon3d_size(seg->prism.base_v * 2));
  x3d_prism3d_get_face(&seg->prism, face_id, wall);
  
  uint16 i;
  
  if((c.x | c.y | c.z) == 0) {
    // Calculate the center of the wall
    X3D_Vex3D_int16 center = { 0, 0, 0 };
    
    for(i = 0; i < wall->total_v; ++i) {
      center.x += wall->v[i].x;
      center.y += wall->v[i].y;
      center.z += wall->v[i].z;
    }
    
    center.x /= wall->total_v;
    center.y /= wall->total_v;
    center.z /= wall->total_v;
    
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
  X3D_WallPortal* to = wall_portals + portal_to;
  
  from->portal_id = portal_to;
  
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
/// Gets the address of a wallportal.
///
/// @param portal_id  - ID of the portal
///
/// @return Address of the wallportal.
///////////////////////////////////////////////////////////////////////////////
X3D_WallPortal* x3d_wallportal_get(uint16 portal_id) {
  return wall_portals + portal_id;
}

