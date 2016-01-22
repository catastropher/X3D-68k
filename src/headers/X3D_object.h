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

#pragma once

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_matrix.h"
#include "X3D_collide.h"

typedef enum {
  X3D_OBJECT_EVENT_CREATE = 0,
  X3D_OBJECT_EVENT_DESTROY = 1,
  
} X3D_ObjectEvents;

typedef struct X3D_ObjectEvent {
  uint8 type;
} X3D_ObjectEvent;

struct X3D_ObjectBase;

typedef struct X3D_ObjectType {
  void (*event_handler)(struct X3D_ObjectBase* object, X3D_ObjectEvent event);
  
  X3D_BoundVolume volume;
} X3D_ObjectType;

typedef struct X3D_ObjectBase {
  X3D_Vex3D_fp16x8 pos;
  X3D_ObjectType* type;
  uint16 id;
} X3D_ObjectBase;

typedef struct X3D_StaticObjectBase {
  X3D_ObjectBase base;
} X3D_StaticObjectBase;

typedef struct X3D_DynamicObjectBase {
  X3D_ObjectBase base;
  X3D_Vex3D_fp8x8 velocity;
  X3D_Mat3x3 mat;
  X3D_Vex3D_angle256 angle;
} X3D_DynamicObjectBase;


#define X3D_MAX_OBJECTS 32
#define X3D_MAX_OBJECT_TYPES 16

typedef struct X3D_ObjectManager {
  uint8 object_pool[X3D_MAX_OBJECTS][256];
  X3D_ObjectType types[X3D_MAX_OBJECT_TYPES];
} X3D_ObjectManager;

typedef void X3D_Object;

///////////////////////////////////////////////////////////////////////////////
/// Gets the whole part of an object's position.
///
/// @param object - object
/// @param v      - position dest
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
static inline void x3d_object_pos(X3D_Object* object, X3D_Vex3D* v) {
  X3D_ObjectBase* obj = object;
  
  v->x = obj->pos.x >> 8;
  v->y = obj->pos.y >> 8;
  v->z = obj->pos.z >> 8;
}

static inline void x3d_dynamicobject_forward_vector(X3D_DynamicObjectBase* object, X3D_Vex3D* dest) {
  dest->x = object->mat.data[2];
  dest->y = object->mat.data[5];
  dest->z = object->mat.data[8];
}

void x3d_objectmanager_init(void);
uint16 x3d_objectmanager_create_object(uint16 type, X3D_Vex3D pos, uint16 seg, X3D_Vex3D dir, fp8x8 speed, X3D_Vex3D_angle256 angle);
void x3d_objectmanager_create_object_type(uint16 type_id, X3D_ObjectType* type);
X3D_DynamicObjectBase* x3d_objectmanager_get_object(uint16 id);
void x3d_object_move(X3D_DynamicObjectBase* obj);

