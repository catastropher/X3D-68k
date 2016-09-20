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

#include "X3D_common.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"
#include "X3D_camera.h"

int16 x3d_scale_by_depth(int16 value, int16 depth, int16 min_depth, int16 max_depth) {
  //if(x3d_rendermanager_get()->wireframe)
  //  return value;

  if(depth >= max_depth)
    return 0;

  return value - ((int32)value * (depth - min_depth) / (max_depth - min_depth));
}

int16 x3d_depth_scale(int16 depth, int16 min_depth, int16 max_depth) {
  if(depth >= max_depth)
    return 0;
  
  if(depth <= min_depth)
    return 0x7FFF;
  
  return 0x7FFF - (((int32)depth - min_depth) << 15) / (max_depth - min_depth);
}

X3D_Color x3d_color_scale_by_depth(X3D_Color color, int16 depth, int16 min_depth, int16 max_depth) {
  uint8 r, g, b;
  x3d_color_to_rgb(color, &r, &g, &b);

  return color;

  return x3d_rgb_to_color(
    x3d_scale_by_depth(r, depth, min_depth, max_depth),
    x3d_scale_by_depth(g, depth, min_depth, max_depth),
    x3d_scale_by_depth(b, depth, min_depth, max_depth)
  );
}

///////////////////////////////////////////////////////////////////////////////
/// Clips a line to a raster region and draws it, if it's visible.
///
/// @param x1     - first x coordinate
/// @param y1     - first y coordinate
/// @param x2     - second x coordinate
/// @param y2     - second y coordinate
/// @param color  - color of the line
/// @param region - region to clip against
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
// void x3d_draw_clipped_line(int16 x1, int16 y1, int16 x2, int16 y2, int16 depth1, int16 depth2, X3D_Color color, X3D_RasterRegion* region) {
// #if 0
//   X3D_RenderManager* renderman = x3d_rendermanager_get();
//   X3D_Vex2D v1 = { x1, y1 };
//   X3D_Vex2D v2 = { x2, y2 };
// 
//   if(depth1 > 1500 && depth2 > 1500)
//     return;
// 
// 
//   if(x3d_rasterregion_clip_line(region, &renderman->stack, &v1, &v2)) {
//     X3D_Color new1 = x3d_color_scale_by_depth(color, depth1, 10, 1500);
//     X3D_Color new2 = x3d_color_scale_by_depth(color, depth2, 10, 1500);
// 
//     x3d_screen_draw_line_grad(v1.x, v1.y, v2.x, v2.y, new1, new2);
//   }
// #endif
// }

void x3d_draw_3d_line(X3D_Vex3D a, X3D_Vex3D b, X3D_CameraObject* cam, X3D_Color color) {
#if 0
  X3D_Vex3D v[2] = { a, b };
  X3D_Vex2D v2d[2];
  
  x3d_camera_transform_points(cam, v, 2, v, v2d);
  
  X3D_Vex2D aa, bb;
  
  if(!(x3d_clip_line_to_near_plane(v, v + 1, v2d, v2d + 1, &aa, &bb, 10) & EDGE_INVISIBLE)) {
    x3d_screen_draw_line(aa.x, aa.y, bb.x, bb.y, color);
  }
#endif
}

