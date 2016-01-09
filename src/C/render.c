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
#include "X3D_prism.h"
#include "X3D_camera.h"
#include "X3D_segment.h"

void x3d_prism3d_render(X3D_Prism3D* prism, X3D_CameraObject* object, X3D_Color color) {
  X3D_Vex2D v[prism->base_v * 2];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(object, &cam_pos);
  
  for(i = 0; i < prism->base_v * 2; ++i) {
    X3D_Vex3D translate = {
      prism->v[i].x - cam_pos.x,
      prism->v[i].y - cam_pos.y,
      prism->v[i].z - cam_pos.z
    };
    
    X3D_Vex3D rot;
    x3d_vex3d_int16_rotate(&rot, &translate, &object->base.mat);
    x3d_vex3d_int16_project(v + i, &rot);
  }
  
  for(i = 0; i < prism->base_v; ++i) {
    uint16 next = (i != prism->base_v - 1 ? i + 1 : 0);
    
    // BASE_A
    x3d_screen_draw_line(
      v[i].x,
      v[i].y,
      v[next].x,
      v[next].y,
      color
    );
    
    // BASE_B
    x3d_screen_draw_line(
      v[i + prism->base_v].x,
      v[i + prism->base_v].y,
      v[next + prism->base_v].x,
      v[next + prism->base_v].y,
      color
    );
    
    // Connecting lines
    x3d_screen_draw_line(
      v[i].x,
      v[i].y,
      v[i + prism->base_v].x,
      v[i + prism->base_v].y,
      color
    );
  }
}

void x3d_polygon3d_render_wireframe_no_clip(X3D_Polygon3D* poly, X3D_CameraObject* object, X3D_Color color) {
  X3D_Vex2D v[poly->total_v];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(object, &cam_pos);
  
  for(i = 0; i < poly->total_v; ++i) {
    X3D_Vex3D translate = {
      poly->v[i].x - cam_pos.x,
      poly->v[i].y - cam_pos.y,
      poly->v[i].z - cam_pos.z
    };
    
    X3D_Vex3D rot;
    x3d_vex3d_int16_rotate(&rot, &translate, &object->base.mat);
    x3d_vex3d_int16_project(v + i, &rot);
  }
  
  for(i = 0; i < poly->total_v; ++i) {
    uint16 next = (i != poly->total_v - 1 ? i + 1 : 0);
    
    // BASE_A
    x3d_screen_draw_line(
      v[i].x,
      v[i].y,
      v[next].x,
      v[next].y,
      color
    );
  }
}

void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color) {
  // Load the segment into the cache
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(id);

  x3d_prism3d_render(&seg->prism, cam, color);
}

