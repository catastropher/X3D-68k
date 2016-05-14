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
#include "X3D_camera.h"
#include "X3D_enginestate.h"


_Bool x3d_line3d_intersect_plane(X3D_Line3D* line, X3D_Plane* plane, X3D_Vex3D* inter, int16* scale) {
  int16 bottom = x3d_vex3d_fp0x16_dot(&plane->normal, &line->dir);
  
  if(bottom == 0)
    return X3D_FALSE;
  
  int32 top = -(x3d_vex3d_int16_dot(&plane->normal, &line->start) - ((int32)plane->d << X3D_NORMAL_BITS));
  int16 t = top / bottom;
  
  if(t < 0)
    return X3D_FALSE;
  
  inter->x = line->start.x + (((int32)line->dir.x * t) >> X3D_NORMAL_BITS);
  inter->y = line->start.y + (((int32)line->dir.y * t) >> X3D_NORMAL_BITS);
  inter->z = line->start.z + (((int32)line->dir.z * t) >> X3D_NORMAL_BITS);
  
  *scale = t;
  
  return X3D_TRUE;
}

_Bool x3d_line3d_intersect_polygon(X3D_Line3D* line, X3D_Polygon3D* poly, X3D_Vex3D* inter) {
  X3D_Plane poly_plane;
  x3d_polygon3d_calculate_plane(poly, &poly_plane);
  
  int16 scale;
  if(!x3d_line3d_intersect_plane(line, &poly_plane, inter, &scale))
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
  X3D_Vex3D dir = { p->x - screen->center.x, p->y - screen->center.y, near_z };
  x3d_vex3d_int16_rotate(&line->dir, &dir, &cam_mat);
  x3d_vex3d_fp0x16_normalize(&line->dir);
  
  x3d_object_pos((X3D_ObjectBase* )cam, &line->start);
}

#include <SDL/SDL.h>

void x3d_line3d_test(X3D_CameraObject* cam) {
  X3D_Vex2D pos;
  X3D_Line3D line;
  SDL_GetMouseState(&pos.x, &pos.y);
  
  x3d_line3d_from_screen_point(&line, &pos, cam, 15);
  
  X3D_Vex3D dir;
  x3d_dynamicobject_forward_vector(&cam->base, &dir);
  
  char buf[1024];
  sprintf(buf, "Dir: { %d, %d, %d } -> { %d, %d, %d }", line.dir.x, line.dir.y, line.dir.z, dir.x, dir.y, dir.z);
  
  SDL_WM_SetCaption(buf, NULL);
  
}

