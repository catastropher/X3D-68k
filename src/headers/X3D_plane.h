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

#pragma once

#include "X3D_common.h"
#include "X3D_matrix.h"

///////////////////////////////////////////////////////////////////////////////
/// A plane with the equation AX + BY + CZ - D = 0
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Plane {
  X3D_Normal3D normal;    ///< Surface normal (A, B, C)
  int16 d;                ///< Distance (D) from the origin to the plane
} X3D_Plane;

typedef struct X3D_Frustum {
  uint16 total_p;
  X3D_Plane* p;
} X3D_Frustum;

typedef enum {
    X3D_PLANE_XY,
    X3D_PLANE_XZ,
    X3D_PLANE_YZ
} X3D_PlaneType;

typedef struct X3D_PlanarProjection {
    X3D_PlaneType plane_type;
    int16 min_x, min_y;
    int16 max_x, max_y;
    X3D_Plane poly_plane;
} X3D_PlanarProjection;

struct X3D_Polygon3D;

void x3d_plane_construct_from_three_points(X3D_Plane* p, X3D_Point3D* p1, X3D_Point3D* p2, X3D_Point3D* p3);
void x3d_plane_print(X3D_Plane* p);
_Bool x3d_plane_guess_orientation(X3D_Plane* plane, X3D_Mat3x3* dest, X3D_Point3D* p);

void x3d_planarprojection_build_from_polygon3d(X3D_PlanarProjection* proj, struct X3D_Polygon3D* poly);
void x3d_planarprojection_project_point(X3D_PlanarProjection* proj, X3D_Vex3D* v, X3D_Vex2D* dest);
void x3d_planarprojection_unproject_point(X3D_PlanarProjection* proj, X3D_Vex2D* src, X3D_Vex3D* dest);

void x3d_frustum_construct_from_polygon3d(X3D_Frustum* frustum, struct X3D_Polygon3D* poly, X3D_Vex3D* p);
_Bool x3d_frustum_point_inside(X3D_Frustum* frustum, X3D_Point3D* p);
void x3d_frustum_construct_from_clipped_polygon3d(struct X3D_Polygon3D* poly, X3D_Frustum* frustum, X3D_Frustum* dest, X3D_Vex3D* cam_pos);

///////////////////////////////////////////////////////////////////////////////
/// Calculates the distance from a point to a plane.
///
/// @param p  - plane
/// @param v  - point
///
/// @return The distance from p to v.
///////////////////////////////////////////////////////////////////////////////
static inline int16 x3d_plane_point_distance(X3D_Plane* p, X3D_Point3D* v) {
  return ((int16)x3d_vex3d_fp0x16_dot(&p->normal, v) - p->d);
}

/// @todo document
static inline void x3d_plane_flip(X3D_Plane* p) {
  p->normal = x3d_vex3d_neg(&p->normal);
  p->d = -p->d;
}

static inline _Bool x3d_plane_point_is_on_normal_facing_side(X3D_Plane* plane, X3D_Vex3D* v) {
    return x3d_plane_point_distance(plane, v) > 0;
}

