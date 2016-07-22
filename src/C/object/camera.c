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
#include "X3D_camera.h"
#include "X3D_enginestate.h"

static void x3d_camera_event_handler(X3D_ObjectBase* obj, X3D_ObjectEvent ev) {
  //X3D_CameraObject* cam = (X3D_CameraObject *)obj;
  
  switch(ev.type) {
    case X3D_OBJECT_EVENT_CREATE:
      printf("Created new camera!\n");
      break;
      
    default:
      break;
  }
}

void x3d_camera_init(void) {
  X3D_ObjectType* cam_type = x3d_objecttype_create(0);
  x3d_objecttype_set_name(cam_type, "xcamera");
  x3d_objecttype_set_event_handler(cam_type, x3d_camera_event_handler);
  x3d_objecttype_set_object_size(cam_type, sizeof(X3D_CameraObject));
  
  x3d_playermanager_get()->player[0] = (X3D_Player) {
    .cam = malloc(sizeof(X3D_CameraObject)),
    .id = 0
  };  
}

///////////////////////////////////////////////////////////////////////////////
/// Transforms a list of points so they are relative to a camera and
///   projected onto the screen.
///
/// @param cam      - camera
/// @param v        - list of points to transform
/// @param total_v  - total numeber of points
/// @param dest3d   - where to write 3D points (optional, NULL if not used)
/// @param dest2d   - where to write projected 2D points
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_camera_transform_points(X3D_CameraObject* cam, X3D_Vex3D* v,
        uint16 total_v, X3D_Vex3D* dest3d, X3D_Vex2D* dest2d) {
  
  X3D_Vex3D cam_pos;                // Position of the camera
  x3d_object_pos(cam, &cam_pos);
  
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    // Translate the point so it's relative to the camera
    X3D_Vex3D translate = {
      v[i].x - cam_pos.x,
      v[i].y - cam_pos.y,
      v[i].z - cam_pos.z
    };
    
    // Rotate the point around the origin
    X3D_Vex3D temp_rot;
    x3d_vex3d_int16_rotate(&temp_rot, &translate, &cam->base.mat);
    
    // Project it onto the screen
    if(dest2d)
      x3d_vex3d_int16_project(dest2d + i, &temp_rot);
    
    // Store the rotated 3D point, if requested
    if(dest3d) {
      *dest3d = temp_rot;
      ++dest3d;
    }
  }
}


void x3d_camera_calculate_shift(X3D_CameraObject* new_cam, X3D_CameraObject* old_cam, X3D_Vex3D* v_from, X3D_Vex3D* v_to) {
  X3D_Vex3D cam_pos;
  x3d_object_pos(old_cam, &cam_pos);
  
  X3D_Vex3D center, c = x3d_vex3d_sub(v_to, &cam_pos);

  c.x += old_cam->shift.x;
  c.y += old_cam->shift.y;
  c.z += old_cam->shift.z;
  
  x3d_vex3d_int16_rotate(&center, &c, &new_cam->base.mat);
  
  X3D_Vex3D pcenter, pc = x3d_vex3d_sub(v_from, &cam_pos);
  
  pc.x += old_cam->shift.x;
  pc.y += old_cam->shift.y;
  pc.z += old_cam->shift.z;
  
  x3d_vex3d_int16_rotate(&pcenter, &pc, &old_cam->base.mat);
  
  new_cam->shift.x = pcenter.x - center.x + old_cam->shift.x;
  new_cam->shift.y = pcenter.y - center.y + old_cam->shift.y;
  new_cam->shift.z = pcenter.z - center.z + old_cam->shift.z;
}

