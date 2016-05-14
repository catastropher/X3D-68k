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

#include "collide/X3D_raytrace.h"
#include "X3D_polygon.h"
#include "X3D_plane.h"


_Bool x3d_line3d_intersect_plane(X3D_Line3D* line, X3D_Plane* plane, X3D_Vex3D* inter) {
  int16 bottom = x3d_vex3d_fp0x16_dot(&plane->normal, &line->dir);
  
  if(bottom == 0)
    return X3D_FALSE;
  
  int32 top = -(x3d_vex3d_int16_dot(&plane->normal, &line->start) - ((int32)plane->d << X3D_NORMAL_SHIFT));
  int16 t = top / bottom;
  
  inter->x = line->start.x + (((int32)line->dir.x * t) >> X3D_NORMAL_SHIFT);
  inter->y = line->start.y + (((int32)line->dir.y * t) >> X3D_NORMAL_SHIFT);
  inter->z = line->start.z + (((int32)line->dir.z * t) >> X3D_NORMAL_SHIFT);
  
  return X3D_TRUE;
}

_Bool x3d_line3d_intersect_polygon(X3D_Line3D* line, X3D_Polygon3D* poly, X3D_Vex3D* inter) {
  X3D_Plane poly_plane;
  x3d_polygon3d_calculate_plane(poly, &poly_plane);
  
  if(!x3d_line3d_intersect_plane(line, &poly_plane, inter))
    return X3D_FALSE;
  
  X3D_Plane planes[poly->total_v];
  X3D_Frustum frustum = {
    .p = planes
  };
  
  x3d_frustum_construct_from_polygon3d(&frustum, poly, &line->start);
  
  return x3d_frustum_point_inside(&frustum, inter);
}


