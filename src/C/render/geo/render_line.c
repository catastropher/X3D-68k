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

#include "X3D_screen.h"
#include "geo/X3D_line.h"
#include "X3D_camera.h"

void x3d_ray3d_render(X3D_Ray3D* ray, X3D_CameraObject* cam, X3D_ColorIndex color) {
  X3D_Ray3D rotated_ray;
  x3d_camera_transform_points(cam, ray->v, 2, rotated_ray.v, NULL);
  
  
  X3D_Ray3D clipped_ray;
  if(x3d_ray3d_clip_to_near_plane(&rotated_ray, &clipped_ray) == X3D_RAY3D_INVISIBLE)
    return;
  
  X3D_Ray2D projected_ray;
  x3d_ray3d_project_to_ray2d(&clipped_ray, &projected_ray);
  
  x3d_screen_draw_line(projected_ray.v[0].x, projected_ray.v[0].y, projected_ray.v[1].x, projected_ray.v[1].y, color);
}

