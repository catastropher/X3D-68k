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
#include "X3D_matrix.h"

#define X3D_MAX_ACTIVE_OBJECTS 10
#define X3D_OBJECT_SIZE 128
#define X3D_MAX_OBJECT_TYPES 20
#define X3D_MAX_CAMERAS 4
#define X3D_MAX_OBJECTS 32

#define X3D_MAX_OBJECT_SEGS 4

#define X3D_OBJECT_NONE 0xFFFF

enum {
  X3D_OBJECT_IN_USE = 1
};

typedef enum {
  X3D_EV_CREATE,
  X3D_EV_DESTROY,
  X3D_EV_VISIBLE,
  X3D_EV_NOTVISIBLE,
  X3D_EV_RENDER
} X3D_EventType;

typedef enum {
  X3D_BOUND_SPHERE,
  X3D_BOUND_CAPSULE
} X3D_BoundType;

typedef struct X3D_BoundVolume {
  X3D_BoundType type;
  
  union {
    struct {
      uint16 radius;
    } sphere;

    struct {
      uint16 radius;
      uint16 height;
    } capsule;
  };
} X3D_BoundVolume;

struct X3D_Frustum;
struct X3D_Segment;
struct X3D_ViewPort;

typedef struct X3D_Event {
  X3D_EventType type;

  union {
    struct {
      struct X3D_Frustum* frustum;
      struct X3D_Segment* segment;
	  struct X3D_ViewPort* viewport;
    } render;
  };

} X3D_Event;

struct X3D_Object;
struct X3D_Context;
struct X3D_Camera;

typedef struct X3D_ObjectType {
  void (*event_handler)(struct X3D_Context* context, struct X3D_Object* obj, X3D_Event ev);
  uint8 wall_behavior;
  X3D_BoundVolume volume;
  Vex3D gravity;

} X3D_ObjectType;

enum {
  X3D_OBJECT_ACTIVE = 1
};

typedef struct Mat3x3_fp0x8 {
} Mat3x3_fp0x8;

typedef struct X3D_Model {
  uint16 bound_sphere;
} X3D_Model;

typedef struct X3D_SegmentPos {
  uint16 segs[X3D_MAX_OBJECT_SEGS];
} X3D_SegmentPos;

enum {
  X3D_COLLIDE_SLIDE,
  X3D_COLLIDE_BOUNCE
};

/// @todo Should be renamed to X3D_BaseObject
typedef struct X3D_Object {
  uint8 flags;
  uint8 wall_behavior;
  uint16 id;
  uint16 type;
  uint16 category;
  Vex3D_fp16x16 pos;
  X3D_Mat3x3_fp0x16 mat;
  Vex3D gravity;

  X3D_BoundVolume volume;
  
  Vex3D_fp16x16 dir;
  int16 speed;

  X3D_SegmentPos seg_pos;

  Vex3D_angle256 angle;

  X3D_Model* unrotated;
  X3D_Model* rotated;
  
  void (*event_handler)(struct X3D_Context* context, struct X3D_Object* obj, X3D_Event ev);
  
  uint8 collide_behavior;
} X3D_Object;

typedef struct X3D_Camera {
  X3D_Object object;
} X3D_Camera;

typedef void* X3D_ObjectBase;

#ifdef __X3D_SHARED__

_Bool x3d_is_object_active(X3D_Object* obj);
_Bool x3d_activate_object(struct X3D_Context* context, X3D_Object* obj);
void x3d_deactivate_object(struct X3D_Context* context, X3D_Object* obj);
X3D_Object* x3d_get_object(struct X3D_Context* context, uint16 id);
X3D_Object* x3d_create_object( struct X3D_Context* context, uint16 object_type, Vex3D pos, Vex3D_angle256 angle, Vex3D_fp0x16 velocity, _Bool active, uint16 seg);
struct X3D_Camera* x3d_create_camera(struct X3D_Context* context, uint16 id, Vex3D pos, Vex3D_angle256 angle);
void x3d_add_object_type(struct X3D_Context* context, uint16 type_id, X3D_ObjectType* type);
void x3d_clear_all_objects(struct X3D_Context* context);


static inline void x3d_object_pos(void* obj, Vex3D* v) {
  X3D_Object* object = (X3D_Object *)obj;
  
  v->x = object->pos.x >> X3D_NORMAL_SHIFT;
  v->y = object->pos.y >> X3D_NORMAL_SHIFT;
  v->z = object->pos.z >> X3D_NORMAL_SHIFT;
}

#else

#endif

