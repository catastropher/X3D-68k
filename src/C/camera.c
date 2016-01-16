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
  X3D_CameraObject* cam = (X3D_CameraObject *)obj;
  
  switch(ev.type) {
    case X3D_OBJECT_EVENT_CREATE:
      printf("Created new camera!\n");
      break;
      
    default:
      break;
  }
}

void x3d_camera_init(void) {
  X3D_ObjectType type_cam = {
    .event_handler = x3d_camera_event_handler
  };
  
  x3d_objectmanager_create_object_type(0, &type_cam);
  uint16 cam_id = x3d_objectmanager_create_object(
    0,
    (X3D_Vex3D) { 0, 0, 0 },
    0,
    (X3D_Vex3D) { 0, 0, 0 },
    0,
    (X3D_Vex3D_angle256) { 0, 0, 0 }
  );
  
  x3d_playermanager_get()->player[0] = (X3D_Player) {
    .cam = x3d_objectmanager_get_object(cam_id),
    .id = 0
  };
  
  
}

