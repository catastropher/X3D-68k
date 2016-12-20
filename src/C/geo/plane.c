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

  // D = (AX + BY + CZ)
  p->d = -(x3d_vex3d_dot(&p->normal, p1) >> X3D_NORMAL_BITS);
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
    float D = -plane->d;

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
    y = x3d_vex3d_neg(&y);
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
    x3d_plane_construct_from_three_points(frustum->p + i, p, poly->v + next, poly->v + i);
  }
  
  // Make sure the planes aren't facing the wrong way (if so we need to flip the normals)
  X3D_Vex3D center;
  x3d_polygon3d_center(poly, &center);
  
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

void x3d_frustum_construct_from_clipped_polygon3d(X3D_Polygon3D* poly, X3D_Frustum* frustum, X3D_Frustum* dest, X3D_Vex3D* cam_pos) {
    X3D_Polygon3D clipped = { .v = alloca(1000) };
    
    x3d_polygon3d_clip_to_frustum(poly, frustum, &clipped);
    x3d_frustum_construct_from_polygon3d(dest, &clipped, cam_pos);
}

static inline void* advance_ptr(void* ptr, size_t dist) {
    return ptr + dist;
}

// Builds a planar projection of a polygon (disregards the least significant axis) and
// normalizes the result
void x3d_planarprojection_build_from_polygon3d(X3D_PlanarProjection* proj, X3D_Polygon3D* poly) {
    X3D_Plane plane;
    
    int16* u_component;
    int16* v_component;
    
    proj->plane_type = x3d_polygon3d_calculate_planetype(poly, &plane, &u_component, &v_component);
    
    int16 min_x = 0x7FFF;
    int16 max_x = -0x7FFF;
    
    int16 min_y = 0x7FFF;
    int16 max_y = -0x7FFF;
    
    uint16 i;
    for(i = 0; i < poly->total_v; ++i) {
        int16 u = *u_component;
        int16 v = *v_component;
        
        min_x = X3D_MIN(min_x, u);
        min_y = X3D_MIN(min_y, v);
        
        max_x = X3D_MAX(max_x, u);
        max_y = X3D_MAX(max_y, v);
        
        u_component = advance_ptr(u_component, sizeof(X3D_Vex3D));
        v_component = advance_ptr(v_component, sizeof(X3D_Vex3D));
    }
    
    proj->min_x = min_x;
    proj->min_y = min_y;
    
    proj->max_x = max_x;
    proj->max_y = max_y;
    proj->poly_plane = plane;
}

void x3d_planarprojection_project_point(X3D_PlanarProjection* proj, X3D_Vex3D* v, X3D_Vex2D* dest) {
    if(proj->plane_type == X3D_PLANE_YZ) {
        dest->x = v->y;
        dest->y = v->z;
    }
    else if(proj->plane_type == X3D_PLANE_XZ) {
        dest->x = v->x;
        dest->y = v->z;
    }
    else {
        dest->x = v->x;
        dest->y = v->y;
    }
    
    dest->x = x3d_units_to_texels(dest->x - proj->min_x);
    dest->y = x3d_units_to_texels(dest->y - proj->min_y);
}

void x3d_planarprojection_unproject_point(X3D_PlanarProjection* proj, X3D_Vex2D* src, X3D_Vex3D* dest) {
    X3D_Vex2D v = { x3d_texels_to_units(src->x) + proj->min_x, x3d_texels_to_units(src->y) + proj->min_y };
    
    X3D_Vex3D_float n = {
        proj->poly_plane.normal.x / 32767.0,
        proj->poly_plane.normal.y / 32767.0,
        proj->poly_plane.normal.z / 32767.0
    };
    
    if(proj->plane_type == X3D_PLANE_YZ) {
        dest->x = -(n.y * v.x + n.z * v.y + proj->poly_plane.d) / n.x;
        dest->y = v.x;
        dest->z = v.y;
    }
    else if(proj->plane_type == X3D_PLANE_XZ) {
        dest->x = v.x;
        dest->y = - ( n.x * v.x + n.z * n.y + proj->poly_plane.d) / n.y;
        dest->z = v.y;
    }
    else {
        dest->x = v.x;
        dest->y = v.y;
        dest->z = - ( n.x * v.x + n.y * v.y + proj->poly_plane.d) / n.z;
    }    
}

