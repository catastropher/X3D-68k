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

#define X3D_MAX_ACTIVE_OBJECTS 10
#define X3D_OBJECT_SIZE 128

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
  Vex3D_fp16x16 pos;
  Mat3x3_fp0x8 mat;

  X3D_Model* unrotated;
  X3D_Model* rotated;
} X3D_Object;

typedef struct X3D_Camera {
  uint8 flags;
  X3D_Object object;
  uint8 fov;
} X3D_Camera;

typedef struct X3D_ObjectManager {
  void* object_data;
  X3D_Object* active_list[X3D_MAX_ACTIVE_OBJECTS];
} X3D_ObjectManager;

