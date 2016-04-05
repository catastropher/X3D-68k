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
#include "X3D_polygon.h"
#include "X3D_trig.h"
#include "X3D_matrix.h"
#include "X3D_clip.h"
#include "X3D_camera.h"
#include "X3D_enginestate.h"


#include "X3D_keys.h"

///////////////////////////////////////////////////////////////////////////////
/// Prints out the points in a polygon (for debugging).
///
/// @param p - polygon
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_print(X3D_Polygon3D* p) {
  x3d_log(X3D_INFO, "X3D_Polygon3D (v = %d)\n", p->total_v);

  uint16 i;
  for(i = 0; i < p->total_v; ++i) {
    x3d_log(X3D_INFO, "\t{%d, %d, %d}\n", p->v[i].x, p->v[i].y, p->v[i].z);
  }

  x3d_log(X3D_INFO, "\n");
}

///////////////////////////////////////////////////////////////////////////////
/// Translates a 3D polygon along a normal vector by a certain amount.
///
/// @param poly - polygon to translate
/// @param dir  - direction to translate in
/// @param dist - distance to move polygon along dir
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_translate_normal(X3D_Polygon3D* poly, X3D_Normal3D* dir, int16 dist) {
  X3D_Vex3D shift = {
    ((int32)dist * dir->x) >> X3D_NORMAL_BITS,
    ((int32)dist * dir->y) >> X3D_NORMAL_BITS,
    ((int32)dist * dir->z) >> X3D_NORMAL_BITS
  };

  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    poly->v[i].x += shift.x;
    poly->v[i].y += shift.y;
    poly->v[i].z += shift.z;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Translates a polygon by a shift vector.
///
/// @param poly   - polygon to shift
/// @param shift  - vector to shift by
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_translate(X3D_Polygon3D* poly, X3D_Vex3D shift) {
  uint16 i;
  for(i = 0; i < poly->total_v; ++i)
    poly->v[i] = x3d_vex3d_add(poly->v + i, &shift);
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the center of a 3D polygon.
///
/// @param poly - poly
/// @param dest - dest for center of the polygon
///
/// @return Nothing.
/// @note This may lead to division by 0 if poly->total_v == 0
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_center(X3D_Polygon3D* poly, X3D_Vex3D* dest) {
  // To prevent division by 0...
  x3d_assert(poly->total_v != 0);
  
  X3D_Vex3D_int32 center = { 0, 0, 0 };

  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    center.x += poly->v[i].x;
    center.y += poly->v[i].y;
    center.z += poly->v[i].z;
  }

  dest->x = center.x / poly->total_v;
  dest->y = center.y / poly->total_v;
  dest->z = center.z / poly->total_v;
}

///////////////////////////////////////////////////////////////////////////////
/// Scales a polygon relative to its center
///
/// @param poly - poly
/// @param scale  - scaling factor in fp8x8 format
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_scale(X3D_Polygon3D* poly, fp8x8 scale) {
  X3D_Vex3D center;
  x3d_polygon3d_center(poly, &center);

  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    poly->v[i].x = (((int32)(poly->v[i].x - center.x) * scale) >> 8) + center.x;
    poly->v[i].y = (((int32)(poly->v[i].y - center.y) * scale) >> 8) + center.y;
    poly->v[i].z = (((int32)(poly->v[i].z - center.z) * scale) >> 8) + center.z;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Reverses the points in a 3D polygon. If it was clockwise, it will now be
///   counter-clockwise.
///
/// @param poly - poly to reverse
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_reverse(X3D_Polygon3D* poly) {
  uint16 i;

  for(i = 0; i < poly->total_v / 2; ++i) {
    X3D_SWAP(poly->v[i], poly->v[poly->total_v - i - 1]);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Constructs a regular 2D polygon (all the sides have the same length).
///
/// @param poly   - poly
/// @param steps  - numbers of steps in the polygon i.e. number of sides
/// @param r      - radius of the polygon
/// @param ang    - angle of the orientation of the polygon
///
/// @todo Apply the fix that was created for x3d_prism3d_construct() because
///       we wind up creating a polygon with unexpected side lengths.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon2d_construct(X3D_Polygon2D* poly, uint16 steps, int16 r, angle256 ang) {
  ufp8x8 angle = (uint16)ang << 8;
  ufp8x8 angle_step = 65536L / steps;

  poly->total_v = steps;

  uint16 i;
  // Construct the polygon
  for(i = 0; i < steps; ++i) {
    poly->v[i].x = mul_fp0x16_by_int16_as_int16(x3d_cos(x3d_uint16_upper(angle)), r);
    poly->v[i].y = mul_fp0x16_by_int16_as_int16(x3d_sin(x3d_uint16_upper(angle)), r);

    angle += angle_step;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Attemps to take a 2D polygon and orient it on a 3D surface.
///
/// @param poly         - 2D polygon
/// @param dest         - dest
/// @param plane        - plane equation of the 3D surface
/// @param top_left     - unused (todo: remove)
/// @param bottom_right - unused (todo: remove)
/// @param mat          - where to put a 3x3 matrix describing the guessed
///                       orientation of the the polygon.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon2d_to_polygon3d(X3D_Polygon2D* poly, X3D_Polygon3D* dest, X3D_Plane* plane, X3D_Vex3D* top_left, X3D_Vex3D* bottom_right, X3D_Mat3x3* mat) {
  // Guess the orientation of the 3D surface
  x3d_plane_guess_orientation(plane, mat, top_left);
  dest->total_v = poly->total_v;
  
  x3d_mat3x3_transpose(mat);
  
  // Rotate each 2D point onto the 3D surface
  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    X3D_Vex3D vv = { poly->v[i].x, poly->v[i].y, 0 };

    x3d_vex3d_int16_rotate(dest->v + i, &vv, mat);
  }
  
  // Transpose it back so that we can store the 3D orientation
  x3d_mat3x3_transpose(mat);
}

///////////////////////////////////////////////////////////////////////////////
/// Rotates a 3D polygon around a center point.
///
/// @param poly   - polygon to rotate
/// @param angle  - euler angles describing the rotation
/// @param center - center to rotate the poly around
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_rotate(X3D_Polygon3D* poly, X3D_Vex3D_angle256 angle, X3D_Vex3D center) {
  X3D_Mat3x3 mat;
  x3d_mat3x3_construct(&mat, &angle);

  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    // Move the point relative to the center
    X3D_Vex3D temp = x3d_vex3d_sub(poly->v + i, &center);
    
    // Rotate it
    x3d_vex3d_int16_rotate(poly->v + i, &temp, &mat);

    // Move it back
    poly->v[i] = x3d_vex3d_add(poly->v + i, &center);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Adds a point to a 2D polygon.
///
/// @param poly - poly
/// @param x    - x coordinate
/// @param y    - y coordinate
///
/// @return Nothing.
/// @note   Make sure there's enough room for the new point!
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon2d_add_point(X3D_Polygon2D* poly, int16 x, int16 y) {
  poly->v[poly->total_v++] = (X3D_Vex2D) { x, y };
}

///////////////////////////////////////////////////////////////////////////////
/// Copies a 3D polygon.
///
/// @param src  - source polygon
/// @param dest - dest polygon
///
/// @return Nothing.
/// @note   Make sure dest is big enough to hold all of src's points!
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_copy(X3D_Polygon3D* src, X3D_Polygon3D* dest) {
  dest->total_v = src->total_v;
  
  uint16 i;
  for(i = 0; i < src->total_v; ++i)
    dest->v[i] = src->v[i];
}

extern int16 render_mode;

///////////////////////////////////////////////////////////////////////////////
/// Renders a 3D polygon.
///
/// @param poly   - polygon to render
/// @param cam    - camera to render through
/// @param parent - parent clipping region to draw inside of
/// @param color  - color of the polygon (will be unused)
/// @param normal - surface normal of the polygon
///
/// @return Nothing.
/// @todo   Refactor this mess!
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon3d_render(X3D_Polygon3D* poly, X3D_CameraObject* cam, X3D_RasterRegion* parent, X3D_Color color, X3D_Vex3D* normal) {
  X3D_Vex3D v3d[poly->total_v];
  X3D_Vex2D v2d[poly->total_v];
  X3D_RasterEdge edges[poly->total_v + 1];
  X3D_Pair pairs[poly->total_v];
  int16 depth_scale[poly->total_v];
  
  x3d_camera_transform_points(cam, poly->v, poly->total_v, v3d, v2d);
  
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  uint16 i;
  uint16 edge_index[poly->total_v];

  void* stack_ptr = x3d_stack_save(&renderman->stack);
  
  int16 min_z = 0x7FFF;
  
  //x3d_segment_point_normal(seg, i, &normal);
    

  X3D_Vex3D d = { 0, 0, 32767 };
  
  for(i = 0; i < poly->total_v; ++i) {
    fp0x16 dot = x3d_vex3d_fp0x16_dot(&d, &normal[i]);
    
    
    dot = X3D_MIN((int32)dot + 8192, 32767);
    
    dot = X3D_MAX(dot, 0);

    if(render_mode != 3)
      depth_scale[i] = dot;//x3d_depth_scale(v3d[i].z, 10, 1500);
  }
  
  if(render_mode == 0) {
  }
  
  for(i = 0; i < poly->total_v; ++i) {
    uint16 a = i;
    uint16 b = (i + 1 < poly->total_v ? i + 1 : 0);
    
    //depth_scale[i] = 0x7FFF;
    
    min_z = X3D_MIN(min_z, v3d[a].z);
    
    pairs[i].val[0] = a;
    pairs[i].val[1] = b;
    
    X3D_Vex3D temp_a = v3d[a], temp_b = v3d[b];
    X3D_Vex2D dest_a, dest_b;

    uint16 res = x3d_clip_line_to_near_plane(&temp_a, &temp_b, v2d + a, v2d + b, &dest_a, &dest_b, x3d_rendermanager_get()->near_z);

    if(!(res & EDGE_INVISIBLE)) {
      x3d_rasteredge_generate(edges + i, dest_a, dest_b, parent, v3d[a].z, v3d[b].z, &renderman->stack, depth_scale[a], depth_scale[b]);
      edges[i].flags |= res;
    }
    else {
      edges[i].flags = res;
    }
    
    edge_index[i] = i;
  }
  
  X3D_ClipContext clip = {
    .stack = &renderman->stack,
    .parent = parent,
    .edges = edges,
    .total_e = poly->total_v,
    .v3d = v3d,
    .v2d = v2d,
    .edge_pairs = pairs,
    .depth_scale = depth_scale,
    .seg = NULL,
    .normal = NULL,
    .edge_index = edge_index,
    .total_edge_index = poly->total_v
  };
  
  min_z = X3D_MAX(min_z, 1);
  
  X3D_RasterRegion r;
  if(x3d_rasterregion_construct_clipped(&clip, &r)) {
    //x3d_rasterregion_fill_zbuf(&r, color, min_z);
    x3d_rasterregion_draw(v2d, poly->total_v, rand(), parent, min_z, normal, v3d);
  }
  
  x3d_stack_restore(&renderman->stack, stack_ptr);
}

///////////////////////////////////////////////////////////////////////////////
/// Removes duplicate points from a 2D polygon.
///
/// @param poly - poly
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon2d_remove_duplicate(X3D_Polygon2D* poly) {
  uint16 i;
  uint16 pos = 0;
  uint16 prev = poly->total_v - 1;
  
  for(i = 0; i < poly->total_v; ++i) {
    if(poly->v[i].x != poly->v[prev].x || poly->v[i].y != poly->v[prev].y) {
      poly->v[pos++] = poly->v[i];
    }
    
    prev = i;
  }
  
  poly->total_v = pos;
}



