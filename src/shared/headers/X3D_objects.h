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

#pragma once

#include "X3D_vector.h"

#define X3D_MAX_ACTIVE_OBJECTS 10
#define X3D_OBJECT_SIZE 128
#define X3D_MAX_OBJECT_TYPES 20
#define X3D_MAX_CAMERAS 4

typedef enum {
  X3D_EV_CREATE,
  X3D_EV_DESTROY,
  X3D_EV_VISIBLE,
  X3D_EV_NOTVISIBLE
} X3D_EventType;

typedef struct X3D_Event {
  X3D_EventType type;
} X3D_Event;

struct X3D_Object;
struct X3D_Context;
struct X3D_Camera;

typedef struct X3D_ObjectType {
  void(*event_handler)(struct X3D_Context* context, struct X3D_Object* obj, X3D_Event ev);

} X3D_ObjectType;

enum {
  X3D_OBJECT_ACTIVE = 1
};

typedef struct Mat3x3_fp0x8 {
} Mat3x3_fp0x8;

typedef struct X3D_Model {
  uint16 bound_sphere;
} X3D_Model;

typedef struct X3D_Object {
  uint8 flags;
  uint16 id;
  uint16 type;
  uint16 category;
  Vex3D_fp16x16 pos;
  Mat3x3_fp0x8 mat;

  X3D_Model* unrotated;
  X3D_Model* rotated;
} X3D_Object;

typedef struct X3D_ObjectManager {
  void* object_data;
  X3D_Object* active_list[X3D_MAX_ACTIVE_OBJECTS];
  X3D_ObjectType types[X3D_MAX_OBJECT_TYPES];

} X3D_ObjectManager;

#ifdef __X3D_SHARED__

_Bool x3d_is_object_active(X3D_Object* obj);
_Bool x3d_activate_object(struct X3D_Context* context, X3D_Object* obj);
void x3d_deactivate_object(struct X3D_Context* context, X3D_Object* obj);
X3D_Object* x3d_get_object(struct X3D_Context* context, uint16 id);
X3D_Object* x3d_create_object(struct X3D_Context* context, uint16 object_type, Vex3D pos, Vex3D_angle256 angle, Vex3D_fp8x8 velocity);
struct X3D_Camera* x3d_create_camera(struct X3D_Context* context, uint16 id, Vex3D pos, Vex3D_angle256 angle);

#else

#endif

