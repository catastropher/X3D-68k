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
} X3D_ObjectType;

typedef struct X3D_ObjectBase {
  X3D_Vex3D_fp16x8 pos;
  X3D_ObjectType* type;
} X3D_ObjectBase;

typedef struct X3D_StaticObjectBase {
  X3D_ObjectBase base;
} X3D_StaticObjectBase;

typedef struct X3D_DynamicObjectBase {
  X3D_ObjectBase base;
  X3D_Vex3D_fp8x8 velocity;
  X3D_Mat3x3 mat;
} X3D_DynamicObjectBase;

typedef void X3D_Object;

static inline void x3d_object_pos(X3D_Object* object, X3D_Vex3D* v) {
  X3D_ObjectBase* obj = object;
  
  v->x = obj->pos.x >> 8;
  v->y = obj->pos.y >> 8;
  v->z = obj->pos.z >> 8;
}

