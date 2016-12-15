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

void x3d_polygon3d_translate_normal(X3D_Polygon3D* poly, X3D_Normal3D* dir, int16 dist) {
    X3D_Vex3D shift;
    x3d_vex3d_fp0x16_mul_by_int16(dir, dist, &shift);
    x3d_polygon3d_translate(poly, shift);
}

void x3d_polygon3d_translate(X3D_Polygon3D* poly, X3D_Vex3D shift) {
    for(int16 i = 0; i < poly->total_v; ++i)
        poly->v[i] = x3d_vex3d_add(poly->v + i, &shift);
}

void x3d_polygon3d_center(X3D_Polygon3D* poly, X3D_Vex3D* dest) {  
    X3D_Vex3D_int32 center = x3d_vex3d_int32_origin();

    for(int16 i = 0; i < poly->total_v; ++i)
        x3d_vex3d_int32_add_vex3d(&center, dest);

    *dest = x3d_vex3d_int32_div_by_int16_as_vex3d(&center, poly->total_v);
}

void x3d_polygon3d_scale(X3D_Polygon3D* poly, fp8x8 scale) {
    X3D_Ray3D ray;
    x3d_polygon3d_center(poly, &ray.v[0]);

    for(int16 i = 0; i < poly->total_v; ++i) {
        x3d_ray3d_set_v(&ray, 0, poly->v + i);
        x3d_ray3d_interpolate_fp8x8(&ray, scale, poly->v + i);
    }
}

void x3d_polygon3d_reverse(X3D_Polygon3D* poly) {
    for(int i = 0; i < poly->total_v / 2; ++i)
        X3D_SWAP(poly->v[i], poly->v[poly->total_v - i - 1]);
}

void x3d_polygon2d_construct(X3D_Polygon2D* poly, uint16 steps, int16 r, angle256 ang) {
    ufp8x8 angle = (uint16)ang << 8;
    ufp8x8 angle_step = 65536L / steps;

    poly->total_v = steps;

    for(int16 i = 0; i < steps; ++i) {
        x3d_vex2d_make_point_on_circle(r, x3d_uint16_upper(angle), poly->v + i);
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

///////////////////////////////////////////////////////////////////////////////
/// Removes duplicate points from a 2D polygon.
///
/// @param poly - poly
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_polygon2d_remove_duplicate_points(X3D_Polygon2D* poly) {
  uint16 i;
  uint16 pos = 0;
  uint16 prev = poly->total_v - 1;

  return;
  
  //x3d_log(X3D_INFO, "Remove duplicate!");
  
  for(i = 0; i < poly->total_v; ++i) {
    if(poly->v[i].x != poly->v[prev].x || poly->v[i].y != poly->v[prev].y) {
      poly->v[pos++] = poly->v[i];
    }
    
    prev = i;
  }
  
  poly->total_v = pos;
}

uint16 x3d_polygon3d_classify_point(X3D_Vex3D* v, int16 near_z) {
  return 0;
  
//   // Left
//   if(v->x < -v->z) {
//     flags |= EDGE_LEFT_CLIPPED;
//   }
// 
//   // Right
//   if(v->x > v->z) {
//     flags |= EDGE_RIGHT_CLIPPED;
//   }
//   
//   if(v->y > v->z / 2) {
//     //printf("INVISIBLE!!!\n");
//     //return EDGE_INVISIBLE;
//     flags |= EDGE_BOTTOM_CLIPPED;
//   }
//   
//   // Top plane
//   if(v->y < -v->z / 2) {
//     //printf("INVISIBLE!!!\n");
//     //return EDGE_INVISIBLE;
//     flags |= EDGE_TOP_CLIPPED;
//   }
// 
//   
//   if(v->z < near_z) {
//     flags |= EDGE_NEAR_CLIPPED | EDGE_INVISIBLE;
//   }
//   
//   return flags;
}

_Bool x3d_polygon3d_frustum_clipped(X3D_Polygon3D* p, int16 near_z) {
  uint16 flags = 0xFFFF;
  
  uint16 i;
  for(i = 0; i < p->total_v; ++i) {
    flags &= x3d_polygon3d_classify_point(p->v + i, near_z);
  }
  
  return flags != 0;
}


_Bool x3d_polygon3d_point_in_near(X3D_Polygon3D* p, uint16 v, int16 near_z) {
  return p->v[v].z >= near_z;
}

void x3d_polygon3d_clip_add_point(X3D_Polygon3D* poly, uint16* u, uint16* v, X3D_Vex3D p, uint16 uu, uint16 vv) {
  poly->v[poly->total_v] = p;
  u[poly->total_v] = uu;
  v[poly->total_v] = vv;
  
  ++poly->total_v;
}

_Bool x3d_polygon3d_clip_to_near_plane(X3D_Polygon3D* poly, X3D_Polygon3D* dest, int16 near_z, uint16* ua, uint16* va, uint16* new_ua, uint16* new_va) {
  int16 next_v;
  int16 v = poly->total_v - 1;

  near_z = 25;
  
  // Ugh, if we're providing UV coordinates for a texture, just make allocate space and
  // make something up :P
  if(ua == NULL) {
    ua = alloca(poly->total_v * sizeof(uint16));
    va = alloca(poly->total_v * sizeof(uint16));
  }
  
  if(x3d_polygon3d_frustum_clipped(poly, near_z))
    return X3D_FALSE;
  
  dest->total_v = 0;
  
  int16 total_clip = 0;
  
  for(next_v = 0; next_v < poly->total_v; v = next_v, ++next_v) {
    _Bool in      = x3d_polygon3d_point_in_near(poly, v, near_z);
    _Bool next_in = x3d_polygon3d_point_in_near(poly, next_v, near_z);

    if(in)
      x3d_polygon3d_clip_add_point(dest, new_ua, new_va, poly->v[v], ua[v], va[v]);
    
    if(in != next_in) {
      int16 in  = abs(poly->v[v].z - near_z);
      int16 out = abs(poly->v[next_v].z - near_z);
      
      int32 t = ((int32)in << 15) / (in + out);
      
      X3D_Vex3D new_p = {
        x3d_linear_interpolate(poly->v[v].x, poly->v[next_v].x, t),
        x3d_linear_interpolate(poly->v[v].y, poly->v[next_v].y, t),
        near_z
      };
      
      int16 new_u = x3d_linear_interpolate(ua[v], ua[next_v], t);
      int16 new_v = x3d_linear_interpolate(va[v], va[next_v], t);
     
      x3d_polygon3d_clip_add_point(dest, new_ua, new_va, new_p, new_u, new_v);
      
      ++total_clip;
    }
  }
  
  return poly->total_v > 2;
}

///////////////////////////////////////////////////////////////////////////////
/// Clips a 3D polygon to a plane.
///
/// @param poly
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_polygon3d_clip_to_plane(X3D_Polygon3D* poly, X3D_Polygon3D* dest, X3D_Plane* plane, uint16* ua, uint16* va, uint16* new_ua, uint16* new_va, uint16* clip) {
  int16 next_v;
  int16 v = poly->total_v - 1;

  dest->total_v = 0;
  
  int16 total_clip = 0;
  int16 dist;
  int16 next_dist = x3d_plane_point_distance(plane, poly->v + v);
  
  for(next_v = 0; next_v < poly->total_v; v = next_v, ++next_v) {
    dist      = next_dist;
    next_dist = x3d_plane_point_distance(plane, poly->v + next_v);
    
    _Bool in      = dist < 0;
    _Bool next_in = next_dist < 0;

    if(in)
      x3d_polygon3d_clip_add_point(dest, new_ua, new_va, poly->v[v], ua[v], va[v]);
    
    if(in != next_in) {
      int16 in  = abs(dist);
      int16 out = abs(next_dist);
      
      int32 t = ((int32)in << 15) / (in + out);
      
      X3D_Vex3D new_p = {
        x3d_linear_interpolate(poly->v[v].x, poly->v[next_v].x, t),
        x3d_linear_interpolate(poly->v[v].y, poly->v[next_v].y, t),
        x3d_linear_interpolate(poly->v[v].z, poly->v[next_v].z, t),
      };
      
      int16 new_u = x3d_linear_interpolate(ua[v], ua[next_v], t);
      int16 new_v = x3d_linear_interpolate(va[v], va[next_v], t);
     
      *clip = dest->total_v;
      
      ++clip;
      
      x3d_polygon3d_clip_add_point(dest, new_ua, new_va, new_p, new_u, new_v);
      
      ++total_clip;
    }
  }
  
  //*clip = 0xFFFF;
  
  return poly->total_v > 2;
}

X3D_Polygon3D* x3d_polygon3d_temp(void) {
    static X3D_Polygon3D p;
    static X3D_Vex3D v[20];
    
    p.total_v = 0;
    p.v = v;
    
    return &p;
}

X3D_PlaneType x3d_polygon3d_calculate_planetype(X3D_Polygon3D* poly, X3D_Plane* plane_dest, int16** plane_u_component, int16** plane_v_component) {
    x3d_polygon3d_calculate_plane(poly, plane_dest);
    
    X3D_Vex3D n = x3d_vex3d_abs(&plane_dest->normal);
    
    if(n.x > n.y && n.x > n.z) {
        *plane_u_component = &poly->v[0].y;
        *plane_v_component = &poly->v[0].z;
        return X3D_PLANE_YZ;
    }
    
    if(n.y > n.x && n.y > n.z) {
        *plane_u_component = &poly->v[0].x;
        *plane_v_component = &poly->v[0].z;
        return X3D_PLANE_XZ;
    }
    
    *plane_u_component = &poly->v[0].x;
    *plane_v_component = &poly->v[0].y;
    return X3D_PLANE_XY;
}








