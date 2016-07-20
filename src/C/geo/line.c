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

#include "geo/X3D_line.h"
#include "X3D_plane.h"
#include "X3D_polygon.h"
#include "X3D_camera.h"
#include "X3D_enginestate.h"
#include "X3D_clip.h"
#include "X3D_camera.h"

_Bool x3d_line3d_intersect_plane(X3D_Line3D* line, X3D_Plane* plane, X3D_Vex3D* inter, int16* scale) {
  int16 bottom = x3d_vex3d_fp0x16_dot(&plane->normal, &line->dir);
  
  if(bottom == 0)
    return X3D_FALSE;
  
  int32 top = -(x3d_vex3d_dot(&plane->normal, &line->start) - ((int32)plane->d << X3D_NORMAL_BITS));
  int32 t = top / bottom;
  
  //x3d_log(X3D_INFO, "Top: %d, %d, %d => %d", plane->normal.x, plane->normal.y, plane->normal.z, plane->d);
  
  if(t > 32767)
    return X3D_FALSE;
  
  if(t <= 0)
    return X3D_FALSE;
  
  inter->x = line->start.x + (((int32)line->dir.x * t) >> X3D_NORMAL_BITS);
  inter->y = line->start.y + (((int32)line->dir.y * t) >> X3D_NORMAL_BITS);
  inter->z = line->start.z + (((int32)line->dir.z * t) >> X3D_NORMAL_BITS);
  
  *scale = t;
  
  return X3D_TRUE;
}

_Bool x3d_line3d_intersect_polygon(X3D_Line3D* line, X3D_Polygon3D* poly, X3D_Vex3D* inter, int16* scale) {
  X3D_Plane poly_plane;
  x3d_polygon3d_calculate_plane(poly, &poly_plane);
  
  if(!x3d_line3d_intersect_plane(line, &poly_plane, inter, scale))
    return X3D_FALSE;
  
  X3D_Plane planes[poly->total_v];
  X3D_Frustum frustum = {
    .p = planes
  };
  
  x3d_frustum_construct_from_polygon3d(&frustum, poly, &line->start);
  
  return x3d_frustum_point_inside(&frustum, inter);
}

void x3d_line3d_from_screen_point(X3D_Line3D* line, X3D_Vex2D* p, X3D_CameraObject* cam, int16 near_z) {
  X3D_Mat3x3 cam_mat = cam->base.mat;
  x3d_mat3x3_transpose(&cam_mat);
  
  X3D_ScreenManager* screen = x3d_screenmanager_get();
  X3D_Vex3D dir = { p->x - screen->center.x, p->y - screen->center.y, x3d_screenmanager_get()->scale_x };
  x3d_vex3d_int16_rotate(&line->dir, &dir, &cam_mat);
  x3d_vex3d_fp0x16_normalize(&line->dir);
  
  x3d_object_pos((X3D_ObjectBase* )cam, &line->start);
}

_Bool x3d_ray3d_partially_in_front_of_near_plane(X3D_Ray3D* ray) {
  return x3d_vex3d_in_front_of_near_plane(ray->v + 0) || x3d_vex3d_in_front_of_near_plane(ray->v + 1);
}

_Bool x3d_ray3d_totally_in_front_of_near_plane(X3D_Ray3D* ray) {
  return x3d_vex3d_in_front_of_near_plane(ray->v + 0) && x3d_vex3d_in_front_of_near_plane(ray->v + 1);
}

void x3d_ray3d_interpolate(X3D_Ray3D* ray, fp0x16 t, X3D_Vex3D* dest) {
  if(t == X3D_FP0x16_MAX) {
    *dest = ray->v[1];
    return;
  }
  
  
  dest->x = x3d_linear_interpolate(ray->v[0].x, ray->v[1].x, t);
  dest->y = x3d_linear_interpolate(ray->v[0].y, ray->v[1].y, t);
  dest->z = x3d_linear_interpolate(ray->v[0].z, ray->v[1].z, t);
}

fp0x16 x3d_ray3d_calculate_near_plane_clip_t(X3D_Ray3D* ray) {
  int16 near_z = x3d_rendermanager_get()->near_z;  
  int16 v0_dist_to_near_plane = abs(ray->v[0].z - near_z);
  int16 v1_dist_to_near_plane = abs(ray->v[1].z - near_z);
  
  int16 n = v0_dist_to_near_plane;
  int16 d = v0_dist_to_near_plane + v1_dist_to_near_plane;
  
  if(div_int16_by_int16_as_fp0x16_would_cause_overflow(n, d)) {
    return X3D_FP0x16_MAX;
  }
  
  return div_int16_by_int16_as_fp0x16(n, d);
}

void x3d_ray3d_clip_to_plane_given_t(X3D_Ray3D* ray, fp0x16 t, X3D_Ray3D* dest) {
  dest->v[0] = ray->v[0];
  x3d_ray3d_interpolate(ray, t, dest->v + 1);
}

X3D_Ray3DClipStatus x3d_ray3d_clip_to_near_plane(X3D_Ray3D* ray, X3D_Ray3D* dest) {
  if(!x3d_ray3d_partially_in_front_of_near_plane(ray))
    return X3D_RAY3D_INVISIBLE;
  
  if(x3d_ray3d_totally_in_front_of_near_plane(ray)) {
    *dest = *ray;
    return X3D_RAY3D_NOT_CLIPPED;
  }
  
  _Bool swap_points_so_v0_in_v1_out = x3d_vex3d_in_front_of_near_plane(ray->v + 1);
  if(swap_points_so_v0_in_v1_out)
    x3d_ray3d_swap_v(ray);

  x3d_assert(ray->v[0].z >= 10 && ray->v[1].z <= 10);
  
  fp0x16 clip_t = x3d_ray3d_calculate_near_plane_clip_t(ray);  
  
  x3d_log(X3D_INFO, "Clip t: %d", clip_t);
  
  x3d_ray3d_clip_to_plane_given_t(ray, clip_t, dest);
  
  if(swap_points_so_v0_in_v1_out) {
    x3d_ray3d_swap_v(ray);
    return X3D_RAY3D_V0_CLIPPED;
  }
  
  return X3D_RAY3D_V1_CLIPPED;
}

void x3d_ray3d_project_to_ray2d(X3D_Ray3D* ray, X3D_Ray2D* dest) {
  x3d_vex3d_int16_project(dest->v + 0, ray->v + 0);
  x3d_vex3d_int16_project(dest->v + 1, ray->v + 1);
}

