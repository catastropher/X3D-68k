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
#include "geo/X3D_line.h"

//#define x3d_log(...) ;

#include <SDL/SDL.h>

#include "render/X3D_util.h"

#if 0
void x3d_raytrace_find_segface(X3D_CameraObject* cam, X3D_Vex2D pos, X3D_Vex3D* hit_pos, int16* hit_seg, int16* hit_face, int16* scale) {
  X3D_Line3D line;
  
  x3d_line3d_from_screen_point(&line, &pos, cam, 15);
  
  X3D_Vex3D dir;
  x3d_dynamicobject_forward_vector(&cam->base, &dir);
  
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  uint16 i;
  
  *hit_face = -1;
  *hit_seg  = -1;
  int16 min_scale = 0x7FFF;
  X3D_Vex3D hit_inter;
  X3D_Vex3D inter;
  
  uint16 s;
  
  X3D_Polygon3D poly = { .v = alloca(1000) };
  
  for(s = 0; s < x3d_segmentmanager_get()->alloc.alloc_offset.size; ++s) {
    X3D_Segment* seg = x3d_segmentmanager_load(s);
    X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
    for(i = 0; i < seg->base.base_v + 2; ++i) {
      if(face[i].portal_seg_face != X3D_FACE_NONE)
        continue;
      
      x3d_prism3d_get_face(&seg->prism, i, &poly);
      
      int16 scale;
      if(x3d_line3d_intersect_polygon(&line, &poly, &inter, &scale)) {
        if(scale < min_scale) {
          min_scale = scale;
          *hit_seg = s;
          *hit_face = i;
          *hit_pos = inter;
        }
      }      
    }
  }
}

extern char hud_status_bar[512];
extern _Bool display_status_bar;

void x3d_line3d_test(X3D_CameraObject* cam) {
  X3D_Vex2D pos;
  SDL_GetMouseState(&pos.x, &pos.y);
  
  int16 hit_face;
  int16 hit_seg;
  X3D_Vex3D hit_inter;
  int16 i;
  int16 scale;
  
  X3D_Polygon3D poly = { .v = alloca(1000) };
  
  x3d_raytrace_find_segface(cam, pos, &hit_inter, &hit_seg, &hit_face, &scale);
  
  if(hit_face != -1 && hit_inter.z != 0) {
    X3D_Vex2D proj;
    X3D_Vex3D rot;
    x3d_camera_transform_points(x3d_playermanager_get()->player[0].cam, &hit_inter, 1, &rot, &proj);
    
    if(rot.z != 0) {
    
      X3D_Polygon2D circle = { .v = alloca(1000) };
      x3d_polygon2d_construct(&circle, 8, 20, 0);
      
      X3D_Segment* seg = x3d_segmentmanager_load(hit_seg);
      X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
      x3d_prism3d_get_face(&seg->prism, hit_face, &poly);
      
      X3D_Vex3D center;
      X3D_Mat3x3 mat;
      x3d_polygon3d_center(&poly, &center);
      
      x3d_polygon2d_to_polygon3d(&circle, &poly, &face[hit_face].plane, &center, &center, &mat);
      
      x3d_polygon3d_translate(&poly, hit_inter);
      
      for(i = 0; i < poly.total_v; ++i) {
        uint16 next = (i + 1) % poly.total_v;
        
        x3d_draw_3d_line(poly.v[i], poly.v[next], x3d_playermanager_get()->player[0].cam, 31);
      }
      
      //x3d_screen_draw_circle(proj.x, proj.y, 10 * x3d_screenmanager_get()->scale_x / rot.z, 31);      
    }
  }
  
  //display_status_bar = X3D_TRUE;
  //sprintf(hud_status_bar, "seg %d, face %d", hit_seg, hit_face);
  
  char buf[1024];
  //sprintf(buf, "Dir: { %d, %d, %d } -> { %d, %d, %d } (hit seg %d, face %d)", line.dir.x, line.dir.y, line.dir.z, hit_inter.x, hit_inter.y, hit_inter.z, hit_seg, hit_face);
  
  //SDL_WM_SetCaption(buf, NULL);
  
}

#endif

