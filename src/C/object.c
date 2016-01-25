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

static uint16 active_objects[X3D_MAX_OBJECTS];
static uint16 total_active_objects;


void x3d_objectmanager_activate_object(uint16 id) {
  active_objects[total_active_objects++] = id;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the address of the object with the given ID.
///
/// @param id - object ID
///
/// @return The address of the object.
///////////////////////////////////////////////////////////////////////////////
X3D_DynamicObjectBase* x3d_objectmanager_get_object(uint16 id) {
  return (X3D_DynamicObjectBase* )&x3d_objectmanager_get()->object_pool[id];
}


///////////////////////////////////////////////////////////////////////////////
/// Initializes the object manager.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_objectmanager_init(void) {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_OBJECTS; ++i) {
    X3D_DynamicObjectBase* obj = x3d_objectmanager_get_object(i);
    
    obj->base.id = i;
  }
}



///////////////////////////////////////////////////////////////////////////////
/// Creates a new object of the given type.
/// 
/// @param type   - object type ID. See @ref
///               x3d_objectmanager_create_object_type().
/// @param pos    - position of the object
/// @param seg    - the segment the object is located in
/// @param dir    - direction the object is moving in
/// @param speed  - speed the object is moving in dir
/// @param angle  - orientation of the object
///
/// @return The ID of the object.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_objectmanager_create_object(uint16 type, X3D_Vex3D pos, uint16 seg, X3D_Vex3D dir, fp8x8 speed, X3D_Vex3D_angle256 angle) {
  static int16 alloc_id = 0;
  
  X3D_ObjectType* obj_type = &x3d_objectmanager_get()->types[type];
  X3D_DynamicObjectBase* obj = x3d_slab_alloc(obj_type->size);
  
  obj->base.type = obj_type;
  obj->base.pos = (X3D_Vex3D_fp16x8) { (int32)pos.x * 256, (int32)pos.y * 256, (int32)pos.z * 256 };
  obj->velocity = (X3D_Vex3D_fp8x8) { ((int32)dir.x * speed) >> 8, ((int32)dir.y * speed) >> 8, ((int32)dir.z * speed) >> 8 };
  obj->angle = angle;
  
  x3d_mat3x3_construct(&obj->mat, &angle);
  
  X3D_ObjectEvent ev = {
    .type = X3D_OBJECT_EVENT_CREATE
  };
  
  obj->base.type->event_handler((X3D_ObjectBase* )obj, ev);
  
  x3d_objectmanager_activate_object(obj->base.id);
  
  return obj->base.id;
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new object type.
///
/// @param type_id  - the type ID that will be associated with the object
/// @param type     - An @ref X3D_ObjectType struct that describes paramets
///                   about the object.
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_objectmanager_create_object_type(uint16 type_id, X3D_ObjectType* type) {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  x3d_assert(type_id < X3D_MAX_OBJECT_TYPES);
  
  objectman->types[type_id] = *type;
}

///////////////////////////////////////////////////////////////////////////////
/// Applies a force to an object.
///
/// @param obj    - object
/// @param force  - force vector
///                   about the object.
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_object_apply_force(X3D_Object* obj, X3D_Vex3D_fp8x8 force) {
  X3D_DynamicObjectBase* object = (X3D_DynamicObjectBase *)obj;
  
  object->velocity.x += force.x;
  object->velocity.y += force.y;
  object->velocity.z += force.z;
  
  /// @todo Should we cap the velocity if it's greater than some value?
}

void x3d_object_move(X3D_DynamicObjectBase* obj) {
  X3D_Vex3D_fp16x8 new_pos = obj->base.pos;
  
  new_pos.x += obj->velocity.x;
  new_pos.y += obj->velocity.y;
  new_pos.z += obj->velocity.z;
  
  X3D_Vex3D_fp0x16 normal = obj->velocity;
  
  if(!normal.x && !normal.y && !normal.z)
    return;
  
  x3d_vex3d_fp0x16_normalize(&normal);
  
  X3D_RayCaster caster;
  x3d_raycaster_init(&caster, 0, new_pos, normal);
  x3d_raycaster_cast(&caster);
  
  if(caster.dist > 50) {
    obj->base.pos = new_pos;
  }
}

/// @todo Document.
void x3d_objectmanager_render_objects(void) {
  uint16 i;
  X3D_ObjectEvent ev = {
    .type = X3D_OBJECT_EVENT_RENDER
  };
  
  for(i = 0; i < 0; ++i) {
    X3D_DynamicObjectBase* obj = x3d_objectmanager_get_object(active_objects[i]);
    
    obj->base.type->event_handler(obj, ev);
  }
}

