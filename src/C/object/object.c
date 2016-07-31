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
#include "X3D_object.h"
#include "X3D_enginestate.h"
#include "X3D_assert.h"
#include "memory/X3D_handle.h"
#include "memory/X3D_alloc.h"
#include "X3D_render.h"
#include "X3D_wallportal.h"

void x3d_objectmanager_init() {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  objectman->total_object_types = 0;
  objectman->types = NULL;
  
  objectman->objects = malloc(sizeof(X3D_ObjectBase *) * X3D_MAX_OBJECTS);
  
  uint16 i;
  for(i = 0; i < X3D_MAX_OBJECTS; ++i)
    objectman->objects[i] = NULL;
}

X3D_ObjectType* x3d_objecttype_create(uint16 obj_type_id) {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  if(obj_type_id >= objectman->total_object_types) {
    objectman->total_object_types = obj_type_id + 1;
    objectman->types = realloc(objectman->types, sizeof(X3D_ObjectType) * objectman->total_object_types);
  }
  
  X3D_ObjectType* type = objectman->types + obj_type_id;
  
  type->event_handler = NULL;
  type->name[0] = '\0';
  
  return type;
}

X3D_ObjectType* x3d_objecttype_get_ptr(uint16 obj_type_id) {
  return x3d_objectmanager_get()->types + obj_type_id;
}

void x3d_object_send_event(X3D_ObjectBase* obj, const X3D_ObjectEvent ev) {
  X3D_ObjectType* type = x3d_objecttype_get_ptr(obj->obj_type);
  type->event_handler(obj, ev);
}

X3D_ObjectBase* x3d_object_create(uint16 obj_type_id, X3D_Vex3D pos) {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  X3D_ObjectType* type = x3d_objecttype_get_ptr(obj_type_id);
  
  // Find a free object
  uint16 i;
  for(i = 0; i < X3D_MAX_OBJECTS; ++i) {
    X3D_ObjectBase* obj = objectman->objects[i];
    
    if(obj == NULL) {
      obj = x3d_slab_alloc(type->size);
      objectman->objects[i] = obj;
      
      if(obj == NULL)
        return NULL;
      
      x3d_object_set_position_from_vex3d(obj, &pos);
      obj->obj_type = obj_type_id;
      x3d_object_send_create_event(obj);
      
      return obj;
    }
  }
  
  return NULL;
}

void x3d_send_frame_events_to_objects(void) {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  uint16 i;
  for(i = 0; i < X3D_MAX_OBJECTS; ++i) {
    if(objectman->objects[i]) {
      x3d_object_send_frame_event(objectman->objects[i]);
    }
  }
}

void x3d_send_render_events_to_objects(X3D_CameraObject* cam) {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  uint16 i;
  for(i = 0; i < X3D_MAX_OBJECTS; ++i) {
    if(objectman->objects[i]) {
      x3d_object_send_render_event(objectman->objects[i], cam);
    }
  }
}

void x3d_object_set_model(X3D_DynamicObjectBase* obj, X3D_Model* model) {
  obj->model = model;
  x3d_model_calculate_boundsphere(model, &obj->bound_sphere);
}


