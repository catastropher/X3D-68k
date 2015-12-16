// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_config.h"
#include "X3D_geo.h"
#include "X3D_render.h"
#include "X3D_frustum.h"

/// @todo document
void x3d_frustum_from_rendercontext(X3D_Frustum* f, X3D_ViewPort* context) {
  // Calculates the normals of the unrotated planes of the view frustum
  int16 w = context->w;
  int16 h = context->h;

  //c->dist = 120;

  X3D_Vex3D_int16 top_left = { -w / 2, -h / 2, context->scale };
  X3D_Vex3D_int16 top_right = { w / 2, -h / 2, context->scale };

  X3D_Vex3D_int16 bottom_left = { -w / 2, h / 2, context->scale };
  X3D_Vex3D_int16 bottom_right = { w / 2, h / 2, context->scale };

  X3D_Vex3D_int16 cam_pos = { 0, 0, 0 };

  // Top plane
  x3d_plane_construct(f->p + 1, &cam_pos, &top_right, &top_left);

  // Bottom plane
  x3d_plane_construct(f->p + 2, &cam_pos, &bottom_left, &bottom_right);

  // Left plane
  x3d_plane_construct(f->p + 3, &cam_pos, &top_left, &bottom_left);

  // Right plane
  x3d_plane_construct(f->p + 4, &cam_pos, &bottom_right, &top_right);

  // Near plane
  x3d_plane_construct(f->p + 5, &bottom_right, &top_right, &top_left);

  f->p[0].normal = (X3D_Vex3D_int16){ 0, 0, INT16_MAX };

  // Hack...
  f->p[0].d = 15;//c->dist - DIST_TO_NEAR_PLANE;

  f->total_p = 5;
}

void x3d_frustum_print(X3D_Frustum* f) {
  uint16 i;

  for(i = 0; i < f->total_p; ++i) {
    printf("normal: %d %d %d, d: %d\n", f->p[i].normal.x, f->p[i].normal.y, f->p[i].normal.z, f->p[i].d);
  }
}

// Takes a 3D polygon and constructs the planes of the view frustum
// such that the points on the polygon form the bounds. Each plane
// consits of two points on the polygon and one on the camera
//
// With any luck, this will remove the need for a 2D polygon clipper!
void x3d_construct_frustum_from_polygon3D(X3D_Polygon3D* poly, X3D_ViewPort* context, X3D_Frustum* dest) {
  int16 i;

  dest->total_p = poly->total_v;

  Vex3D cam_pos = { 0, 0, 0 };//{ context->cam.pos.x >> X3D_NORMAL_SHIFT, context->cam.pos.y >> X3D_NORMAL_SHIFT, context->cam.pos.z >> X3D_NORMAL_SHIFT };

  for(i = 0; i < poly->total_v; i++) {
    uint16 next_point = i + 1;

    if(next_point == poly->total_v)
      next_point = 0;

    x3d_plane_construct(&dest->p[i], &cam_pos, &poly->v[next_point], &poly->v[i]);

  }
}

