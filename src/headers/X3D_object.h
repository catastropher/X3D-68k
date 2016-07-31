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
#include "memory/X3D_handle.h"
#include "geo/X3D_model.h"
#include "level/X3D_level.h"

typedef enum {
  X3D_OBJECT_EVENT_CREATE = 0,
  X3D_OBJECT_EVENT_DESTROY = 1,
  X3D_OBJECT_EVENT_RENDER = 2,
  X3D_OBJECT_EVENT_FRAME = 3
  
} X3D_ObjectEvents;

struct X3D_CameraObject;
struct X3D_DisplayLineList;
struct X3D_RasterRegion;

typedef struct X3D_ObjectEvent {
  uint8 type;
  
  //union {
    struct {
      struct X3D_CameraObject* cam;
      struct X3D_DisplayLineList* list;
      struct X3D_RasterRegion* region;
    } render_event;
  //};
  
} X3D_ObjectEvent;

struct X3D_ObjectBase;

#define X3D_OBJECTTYPE_MAX_NAME_LENGTH 20

typedef struct X3D_ObjectType {
  char name[X3D_OBJECTTYPE_MAX_NAME_LENGTH];
  void (*event_handler)(struct X3D_ObjectBase* object, const X3D_ObjectEvent event);
  uint16 size;
} X3D_ObjectType;

typedef struct X3D_ObjectBase {
  X3D_Vex3D_fp16x8 pos;
  uint16 obj_type;
  X3D_Handle handle;
  uint16 frame;
  uint16 seg;
} X3D_ObjectBase;

typedef struct X3D_StaticObjectBase {
  X3D_ObjectBase base;
} X3D_StaticObjectBase;

typedef struct X3D_DynamicObjectBase {
  X3D_ObjectBase base;
  X3D_Vex3D_fp8x8 velocity;
  X3D_Mat3x3 mat;
  X3D_Vex3D_angle256 angle;
  X3D_Model* model;
  X3D_BoundSphere bound_sphere;
  X3D_LEVEL_SEG current_seg;
  _Bool on_floor;
} X3D_DynamicObjectBase;


#define X3D_MAX_OBJECTS 32

typedef struct X3D_ObjectManager {
  uint16 total_object_types;
  X3D_ObjectType* types;
  X3D_ObjectBase** objects;
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

void x3d_objectmanager_init();
X3D_ObjectType* x3d_objecttype_create(uint16 obj_type_id);
X3D_ObjectBase* x3d_object_create(uint16 obj_type_id, X3D_Vex3D pos);
void x3d_object_send_event(X3D_ObjectBase* obj, const X3D_ObjectEvent ev);

void x3d_send_frame_events_to_objects(void);
void x3d_send_render_events_to_objects(struct X3D_CameraObject* cam);
void x3d_object_set_model(X3D_DynamicObjectBase* obj, X3D_Model* model);

static inline void x3d_objecttype_set_event_handler(X3D_ObjectType* type, void (*event_handler)(struct X3D_ObjectBase* object, const X3D_ObjectEvent event)) {
  type->event_handler = event_handler;
}

static inline void x3d_objecttype_set_name(X3D_ObjectType* type, const char* name) {
  strcpy(type->name, name);
}

static inline void x3d_objecttype_set_object_size(X3D_ObjectType* type, uint16 size) {
  type->size = size;
}


static inline void x3d_object_set_position_from_vex3d(X3D_ObjectBase* obj, X3D_Vex3D* pos) {
  obj->pos.x = (int32)pos->x << 8;
  obj->pos.y = (int32)pos->y << 8;
  obj->pos.z = (int32)pos->z << 8;
}

static inline void x3d_object_send_create_event(X3D_ObjectBase* obj) {
  x3d_object_send_event(obj, (X3D_ObjectEvent) { .type = X3D_OBJECT_EVENT_CREATE });
}

static inline void x3d_object_send_frame_event(X3D_ObjectBase* obj) {
  x3d_object_send_event(obj, (X3D_ObjectEvent) { .type = X3D_OBJECT_EVENT_FRAME });
}

static inline void x3d_object_send_render_event(X3D_ObjectBase* obj, struct X3D_CameraObject* cam) {
  x3d_object_send_event(obj, (X3D_ObjectEvent) { .type = X3D_OBJECT_EVENT_RENDER, .render_event.cam = cam });
}

static inline void x3d_object_set_on_floor_status(X3D_DynamicObjectBase* obj, _Bool status) {
  obj->on_floor = status;
}

