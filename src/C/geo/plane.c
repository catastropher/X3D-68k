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

#include <math.h>

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_plane.h"
#include "X3D_matrix.h"
#include "X3D_polygon.h"

#include "geo/X3D_point.h"

void x3d_plane_construct_from_three_points(X3D_Plane* p, X3D_Point3D* p1, X3D_Point3D* p2, X3D_Point3D* p3) {
  // Calculate the normal of the plane
  X3D_Vex3D v1 = x3d_vex3d_sub(p1, p2);
  X3D_Vex3D v2 = x3d_vex3d_sub(p3, p2);

  x3d_vex3d_fp0x16_cross(&p->normal, &v1, &v2);

  // For some reason, the 68k is getting a vector that's pointing the wrong way???
#ifdef __68k__
  p->normal = x3d_vex3d_neg(&p->normal);
#endif

  // D = (AX + BY + CZ)
  p->d = x3d_vex3d_dot(&p->normal, p1) >> X3D_NORMAL_BITS;
}

///////////////////////////////////////////////////////////////////////////////
/// Prints out the equation of a plane (for debugging).
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_plane_print(X3D_Plane* p) {
  x3d_log(X3D_INFO, "X3D_Plane\n\tNormal: {%d, %d, %d}\n\tD: %d\n",
    p->normal.x, p->normal.y, p->normal.z, p->d);
}

///////////////////////////////////////////////////////////////////////////////
/// Attemps to guess the orientation of a plane based on its plane equation (
///   assumes the plane has no roll i.e. the x axis is always horizontal.
///
/// @param plane  - plane
/// @param dest   - dest orientation of the plane (a 3x3 matrix)
/// @param p      - a point of the plane
///
/// @return Always true.
/// @todo   Remove return value and rewrite to use fixed-point.
/// @note   This function is very expensive because it uses floating point.
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_plane_guess_orientation(X3D_Plane* plane, X3D_Mat3x3* dest, X3D_Point3D* p) {
  X3D_Vex3D x, y, z = plane->normal;

  if(plane->normal.z != 0) {
    float A = plane->normal.x / 32768.0;
    float B = plane->normal.y / 32768.0;
    float C = plane->normal.z / 32768.0;
    float D = plane->d;

    float u = 10000;
    
    if(plane->normal.z > 0)
      u = -u;
    
    float v = p->z - (-B * p->y + D - A * (p->x + u)) / C;

    float len = sqrt(u * u + v * v);

    u /= len;
    v /= len;

    x.x = u * 32767;
    x.z = -v * 32767;
    x.y = 0;

    x3d_vex3d_fp0x16_cross(&y, &z, &x);    
  }
  else {
    // If this is the case, they're likely on the ceiling, so just
    // pick any orientation
    x.x = 32767;
    x.y = 0;
    x.z = 0;
    
    y.x = 0;
    y.y = 0;
    y.z = 32767;    
  }
  
  x3d_mat3x3_set_column(dest, 0, &x);
  x3d_mat3x3_set_column(dest, 1, &y);
  x3d_mat3x3_set_column(dest, 2, &z);
  
  return X3D_TRUE;  
}

/// @todo document
void x3d_frustum_construct_from_polygon3d(X3D_Frustum* frustum, X3D_Polygon3D* poly, X3D_Vex3D* p) {
  uint16 i;
  for(i = 0; i < poly->total_v; ++i) {
    uint16 next = (i + 1 < poly->total_v ? i + 1 : 0);
    x3d_plane_construct_from_three_points(frustum->p + i, p, poly->v + i, poly->v + next);
  }
  
  // Make sure the planes aren't facing the wrong way (if so we need to flip the normals)
  X3D_Vex3D center;
  x3d_polygon3d_center(poly, &center);
  
  if(x3d_plane_point_distance(frustum->p, &center) > 0) {
    for(i = 0; i < poly->total_v; ++i)
      x3d_plane_flip(frustum->p + i);
  }
  
  frustum->total_p = poly->total_v;
}

/// @todo document
_Bool x3d_frustum_point_inside(X3D_Frustum* frustum, X3D_Point3D* p) {
  uint16 i;
  for(i = 0; i < frustum->total_p; ++i) {
    if(x3d_plane_point_distance(frustum->p + i, p) > 0)
      return X3D_FALSE;
  }
  
  return X3D_TRUE;
}

