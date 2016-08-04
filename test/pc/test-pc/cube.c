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

#include "X3D.h"

#include "render/geo/X3D_render_model.h"
#include "X3D_collide.h"

static X3D_Model cube_model;

#define CUBE_TYPE_ID 1

typedef struct X3D_CubeObject {
  X3D_DynamicObjectBase base;
  X3D_Color color;
} X3D_CubeObject;

extern X3D_Level* global_level;

static void cube_handler(X3D_ObjectBase* obj, const X3D_ObjectEvent ev) {
  X3D_CubeObject* cube = (X3D_CubeObject *)obj;
  X3D_Vex3D pos;
  x3d_object_pos(cube, &pos);
  
  static int16 angle = 0;
  
  X3D_Prism3D prism = { .v = alloca(1000) };
  x3d_prism3d_construct(&prism, 8, 800, 800, (X3D_Vex3D_angle256) { angle, angle, 0 });
  
  X3D_LevelSegment* seg = x3d_level_get_segmentptr(global_level, 0);
  
  switch(ev.type) {
    case X3D_OBJECT_EVENT_CREATE:
      cube->color = 31;
      cube->base.segs[0] = 0;
      break;
    
    case X3D_OBJECT_EVENT_FRAME:
      if(x3d_key_down(X3D_KEY_8)) {
        cube->base.velocity.y -= 512 * 32;
      }
      
      if(x3d_key_down(X3D_KEY_0)) {
        ++angle;
        x3d_levelsegment_update_geometry(global_level, seg, &prism);
      }
      
      x3d_object_move(&cube->base);
      break;
      
    case X3D_OBJECT_EVENT_RENDER:
      x3d_model_render(cube->base.model, ev.render_event.cam, cube->color, cube->base.angle, pos);
      break;
  }
}

void init_cube(void) {
  x3d_model_create_dynamically_allocated_model(&cube_model);
  
  X3D_Prism3D cube_prism = { .v = alloca(1000) };
  x3d_prism3d_construct(&cube_prism, 4, 100, 100, (X3D_Vex3D_angle256) { 0, 0, 0 });
  x3d_model_add_prism3d(&cube_model, &cube_prism, x3d_vex3d_make(0, 0, 0));
  
  X3D_ObjectType* cube_type = x3d_objecttype_create(CUBE_TYPE_ID);
  x3d_objecttype_set_object_size(cube_type, sizeof(X3D_CubeObject));
  x3d_objecttype_set_name(cube_type, "xcube");
  x3d_objecttype_set_event_handler(cube_type, cube_handler);
  
  X3D_DynamicObjectBase* box_obj = x3d_object_create(CUBE_TYPE_ID, x3d_vex3d_make(0, 0, 0));
  x3d_object_set_model(box_obj, &cube_model);
  //box_obj->velocity.z = 256 * 8;
  //box_obj->velocity.y = 256 * 8;
}

