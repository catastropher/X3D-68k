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
#include "X3D_segment.h"
#include "X3D_render.h"
#include "X3D_wallportal.h"

static uint16 active_objects[X3D_MAX_OBJECTS];
static uint16 total_active_objects;


void x3d_objectmanager_activate_object(uint16 id) {
  active_objects[total_active_objects++] = id;
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
/// @return A handle to the object.
///////////////////////////////////////////////////////////////////////////////
X3D_Handle x3d_object_create(uint16 type, X3D_Vex3D pos, uint16 seg, X3D_Vex3D dir, fp8x8 speed, X3D_Vex3D_angle256 angle) {
  X3D_ObjectType* obj_type = &x3d_objectmanager_get()->types[type];
  X3D_DynamicObjectBase* obj = x3d_slab_alloc(obj_type->size);
  
  obj->base.type = obj_type;
  obj->base.pos = (X3D_Vex3D_fp16x8) { (int32)pos.x * 256, (int32)pos.y * 256, (int32)pos.z * 256 };
  obj->velocity = (X3D_Vex3D_fp8x8) { ((int32)dir.x * speed) >> 8, ((int32)dir.y * speed) >> 8, ((int32)dir.z * speed) >> 8 };
  obj->angle = angle;
  obj->base.seg = seg;
  
  x3d_mat3x3_construct(&obj->mat, &angle);
  
  X3D_ObjectEvent ev = {
    .type = X3D_OBJECT_EVENT_CREATE
  };
  
  obj->base.type->event_handler((X3D_ObjectBase* )obj, ev);
  
  X3D_Handle handle = x3d_handle_add(obj);
  
  x3d_uncompressedsegment_add_object(seg, handle);
  
  //x3d_objectmanager_activate_object(obj->base.id);
  
  return handle;
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new object type.
///
/// @param type_id  - the type ID that will be associated with the object
/// @param type     - An @ref X3D_ObjectType struct that describes paramets
///                   about the object.
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_object_create_type(uint16 type_id, X3D_ObjectType* type) {
  X3D_ObjectManager* objectman = x3d_objectmanager_get();
  
  x3d_assert(type_id < X3D_MAX_OBJECT_TYPES);
  
  objectman->types[type_id] = *type;
}

///////////////////////////////////////////////////////////////////////////////
/// Applies a force to an object.
///
/// @param obj    - object
/// @param force  - force vector
///
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
  x3d_raycaster_init(&caster, obj->base.seg, new_pos, normal);
  x3d_raycaster_cast(&caster);
  
  X3D_Segment* seg = x3d_segmentmanager_load(obj->base.seg);
  
  uint16 portals[32];
  uint16 total_p = 0;//x3d_wall_get_wallportals(caster.hit_face, portals);
  
  //obj->base.pos = new_pos;  ///debug
  //return;
  
  if(caster.dist > (total_p == 1 ? 0 : 0) && caster.inside) {
    X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg) + x3d_segfaceid_face(caster.hit_face);
    
    if(face->portal_seg_face != X3D_FACE_NONE || caster.dist > 10 || total_p == 1)
      obj->base.pos = new_pos;
  }
  else {
    printf("Segment before: %d\n", obj->base.seg);
    
    x3d_raycaster_init(&caster, obj->base.seg, obj->base.pos, normal);
    x3d_raycaster_cast(&caster);
    
    X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg) + x3d_segfaceid_face(caster.hit_face);
    
    total_p = 0;//x3d_wall_get_wallportals(caster.hit_face, portals);
    
    x3d_log(X3D_INFO, "Here!");
    
    if(total_p != 0) {
#if 0
      X3D_Vex3D pos = {
        new_pos.x >> 8,
        new_pos.y >> 8,
        new_pos.z >> 8,
      };
      
      x3d_log(X3D_INFO, "Move through portal");
      
      X3D_WallPortal* portal = NULL;//x3d_wallportal_get(portals[0]);
      
      X3D_Vex3D portal_pos;
      x3d_wallportal_transform_point(portal, &pos, &portal_pos);
      
      obj->base.pos.x = (int32)portal_pos.x << 8;
      obj->base.pos.y = (int32)portal_pos.y << 8;
      obj->base.pos.z = (int32)portal_pos.z << 8;
      
      obj->base.seg = x3d_segfaceid_seg(x3d_wallportal_get(portal->portal_id)->face);
      
      X3D_Mat3x3 new_mat;
      x3d_mat3x3_mul(&new_mat, &portal->transform, &obj->mat);
      
      obj->mat = new_mat;
      
      
      x3d_mat3x3_transpose(&new_mat);
      x3d_mat3x3_extract_angles(&new_mat, &obj->angle);
      
    }
    else if(face->portal_seg_face != X3D_FACE_NONE) {
      obj->base.pos = new_pos;
      obj->base.seg = x3d_segfaceid_seg(face->portal_seg_face);
    
      printf("Segment: %d\n", obj->base.seg);
    #endif
    }
  }
}

