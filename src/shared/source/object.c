/* This file is part of X3D.
*
* X3D is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* X3D is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with X3D. If not, see <http://www.gnu.org/licenses/>.
*/

#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_engine.h"
#include "X3D_object.h"
#include "X3D_segment.h"

#define X3D_MAX_OBJECTS 32

enum {
  X3D_OBJECT_IN_USE = 1
};

_Bool x3d_is_object_active(X3D_Object* obj) {
  return obj->flags & X3D_OBJECT_ACTIVE;
}

_Bool x3d_activate_object(X3D_Context* context, X3D_Object* obj) {
  uint16 i;
  X3D_ObjectManager* manager = &context->object_manager;


  for(i = 0; i < X3D_MAX_ACTIVE_OBJECTS; ++i) {
    if(manager->active_list[i] == NULL) {
      manager->active_list[i] = obj;
      obj->flags |= X3D_OBJECT_ACTIVE;
      return TRUE;
    }
  }

  return FALSE;
}

void x3d_deactivate_object(X3D_Context* context, X3D_Object* obj) {
  uint16 i;
  X3D_ObjectManager* manager = &context->object_manager;


  for(i = 0; i < X3D_MAX_ACTIVE_OBJECTS; ++i) {
    if(manager->active_list[i] == obj) {
      manager->active_list[i] = NULL;
      obj->flags &= ~X3D_OBJECT_ACTIVE;
    }
  }
}

X3D_Object* x3d_get_object(X3D_Context* context, uint16 id) {
  return context->object_manager.object_data + id * X3D_OBJECT_SIZE;
}

X3D_Object* x3d_create_object(X3D_Context* context, uint16 object_type, Vex3D pos, Vex3D_angle256 angle, Vex3D_fp0x16 velocity, _Bool active, uint16 seg) {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_OBJECTS; ++i) {
    X3D_Object* object = x3d_get_object(context, i);
    
    if((object->flags & X3D_OBJECT_IN_USE) == 0) {
      object->flags |= X3D_OBJECT_IN_USE;
      
      object->pos.x = (int32)pos.x << X3D_NORMAL_SHIFT;
      object->pos.y = (int32)pos.y << X3D_NORMAL_SHIFT;
      object->pos.z = (int32)pos.z << X3D_NORMAL_SHIFT;
      
      object->dir.x = 0;//velocity.x;
      object->dir.y = 0;//velocity.y;
      object->dir.z = 0;//velocity.z;
      
      if(active) {
        x3d_activate_object(context, object);
      }
      
      object->event_handler = context->object_manager.types[object_type].event_handler;
      object->wall_behavior = context->object_manager.types[object_type].wall_behavior;
      object->volume = context->object_manager.types[object_type].volume;
      object->gravity = context->object_manager.types[object_type].gravity;

      X3D_Event ev = {
        .type = X3D_EV_CREATE
      };
      
      object->event_handler(context, object, ev);
      
      x3d_add_object_to_segment(x3d_get_segment(context, seg), i);
      
      return object;
    }
  }
}

X3D_Camera* x3d_create_camera(X3D_Context* context, uint16 id, Vex3D pos, Vex3D_angle256 angle) {

}


void x3d_init_objectmanager(X3D_Context* context) {
  uint16 i;
  
  context->object_manager.object_data = x3d_malloc(X3D_OBJECT_SIZE * X3D_MAX_OBJECTS);
  
  for(i = 0; i < X3D_MAX_OBJECTS; ++i) {
    X3D_Object* obj = x3d_get_object(context, i);
    
    obj->flags = 0;
    obj->id = i;
  }
  
  for(i = 0; i < X3D_MAX_ACTIVE_OBJECTS; ++i) {
    context->object_manager.active_list[i] = NULL;
  }
}

void x3d_add_object_type(X3D_Context* context, uint16 type_id, X3D_ObjectType* type) {
  context->object_manager.types[type_id] = *type;
}

void x3d_clear_all_objects(X3D_Context* context) {
  x3d_free(context->object_manager.object_data);
  x3d_init_objectmanager(context);
  
  uint16 i, d;
  
  for(i = 0; i < x3d_get_total_segments(context); ++i) {
    X3D_Segment* seg = x3d_get_segment(context, i);
    
    for(d = 0; d < X3D_MAX_OBJECTS_IN_SEGMENT; ++d) {
      seg->objects[d] = X3D_OBJECT_NONE;
    }
  }
}

