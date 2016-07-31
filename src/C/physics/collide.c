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
#include "X3D_enginestate.h"
#include "X3D_collide.h"
#include "X3D_plane.h"
#include "X3D_assert.h"
#include "X3D_vector.h"

#include "X3D_common.h"
#include "X3D_object.h"
#include "geo/X3D_model.h"

extern X3D_Level* global_level;

_Bool x3d_boundsphere_intersec(X3D_BoundSphere* a, X3D_BoundSphere* b) {
  return x3d_vex3d_distance(&a->center, &b->center) < a->radius + b->radius;
}

_Bool x3d_boundsphere_inside_levelsegment(X3D_BoundSphere* sphere, X3D_LevelSegment* seg, X3D_LevelSegFace** hit_face, int16* hit_dist) {
  X3D_LevelSegFace* faces = x3d_levelsegment_get_face_attributes(global_level, seg);
  
  *hit_dist = 10000;
  
  uint16 i;
  for(i = 0; i < seg->base_v + 2; ++i) {
    int16 dist = x3d_plane_point_distance(&faces[i].plane, &sphere->center);
    
    if(dist < *hit_dist) {
      *hit_dist = dist;
      *hit_face = faces + i;
    }
  }
  
  return *hit_dist >= sphere->radius;
}

void x3d_object_reverse_velocity(X3D_DynamicObjectBase* obj, X3D_Vex3D_fp8x8* velocity, X3D_LevelSegFace* hit_face, int16 hit_dist) {
  X3D_Vex3D vel = { velocity->x, velocity->y, velocity->z };
  
  int16 mag = x3d_vex3d_int16_mag(&vel);
  x3d_vex3d_fp0x16_normalize(&vel);
  
  int32 dot = x3d_vex3d_fp0x16_dot(&vel, &hit_face->plane.normal);
  
  X3D_Vex3D proj = {
    (hit_face->plane.normal.x * dot) >> 15,
    (hit_face->plane.normal.y * dot) >> 15,
    (hit_face->plane.normal.z * dot) >> 15,
  };
  
  velocity->x -= 2 * (int32)mag * proj.x / 32768;
  velocity->y -= 2 * (int32)mag * proj.y / 32768;
  velocity->z -= 2 * (int32)mag * proj.z / 32768;
  
  //velocity->x = velocity->x - (((((int32)hit_face->plane.normal.x * dot) >> 15) * mag) >> 8);
  //velocity->y = velocity->y - (((((int32)hit_face->plane.normal.y * dot) >> 15) * mag) >> 8);
  //velocity->z = velocity->z - (((((int32)hit_face->plane.normal.z * dot) >> 15) * mag) >> 8);
}

void x3d_velocity_apply_friction(X3D_Vex3D_fp8x8* velocity);

_Bool x3d_object_keep_velocity_of_sliding_object(X3D_DynamicObjectBase* obj, X3D_Vex3D_fp8x8* velocity, X3D_LevelSegFace* hit_face) {
   X3D_Vex3D vel = { velocity->x, velocity->y, velocity->z };
  
  int16 mag = x3d_vex3d_int16_mag(&vel);
  x3d_vex3d_fp0x16_normalize(&vel);
  
  int32 dot = -x3d_vex3d_fp0x16_dot(&vel, &hit_face->plane.normal);
  
  X3D_Vex3D proj = {
    (hit_face->plane.normal.x * dot) >> 15,
    (hit_face->plane.normal.y * dot) >> 15,
    (hit_face->plane.normal.z * dot) >> 15,
  };

  velocity->x += 1 * (int32)mag * proj.x / 32768;
  velocity->y += 1 * (int32)mag * proj.y / 32768;
  velocity->z += 1 * (int32)mag * proj.z / 32768;
  
  int16 new_mag = x3d_vex3d_int16_mag(velocity);
  
  if(new_mag < 256) {
    return X3D_FALSE;
  }
  
  x3d_velocity_apply_friction(velocity);
  
  return X3D_TRUE;
  
//   X3D_Vex3D proj = {
//     (velocity->x * dot) / 32768,
//     (velocity->y * dot) / 32768,
//     (velocity->z * dot) / 32768,
//   };
//   
//   velocity->x = (int32)proj.x;
//   velocity->y = (int32)proj.y;
//   velocity->z = (int32)proj.z;
}

fp8x8 x3d_scaler_apply_friction(fp8x8 val, int16 friction) {
  if(X3D_SIGNOF(val - friction) != X3D_SIGNOF(val))
    return 0;
  
  return val - friction;
}

void x3d_velocity_apply_friction(X3D_Vex3D_fp8x8* velocity) {
  int32 friction_speed = 256;
  
  X3D_Vex3D vel = *velocity;
  x3d_vex3d_fp0x16_normalize(&vel);
  
  X3D_Vex3D friction = {
    (friction_speed * vel.x) >> 15,
    (friction_speed * vel.y) >> 15,
    (friction_speed * vel.z) >> 15
  };
  
  velocity->x = x3d_scaler_apply_friction(velocity->x, friction.x);
  velocity->y = x3d_scaler_apply_friction(velocity->y, friction.y);
  velocity->z = x3d_scaler_apply_friction(velocity->z, friction.z);
}

#include <SDL/SDL.h>

void x3d_object_move(X3D_DynamicObjectBase* obj) {
  X3D_Vex3D_fp16x8 new_pos;
  
  obj->velocity.y += 512 * 2;
  //obj->velocity.z += 512 * 2;
  
  new_pos.x = obj->base.pos.x + obj->velocity.x;
  new_pos.y = obj->base.pos.y + obj->velocity.y;
  new_pos.z = obj->base.pos.z + obj->velocity.z;
  
  X3D_BoundSphere sphere = { .radius = obj->bound_sphere.radius, .center = x3d_vex3d_make(
    new_pos.x >> 8,
    new_pos.y >> 8,
    new_pos.z >> 8
  )};
  
  
  X3D_LevelSegment* segment = x3d_level_get_segmentptr(global_level, obj->current_seg);
  int16 hit_dist;
  X3D_LevelSegFace* hit_face;
  
  char title[1024];
  sprintf(title, "{ %f, %f, %f } -> on_floor = %d", obj->velocity.x / 256.0, obj->velocity.y / 256.0, obj->velocity.z / 256.0, obj->on_floor);
  SDL_WM_SetCaption(title, NULL);
  
  obj->on_floor = X3D_FALSE;
  
  if(!x3d_boundsphere_inside_levelsegment(&sphere, segment, &hit_face, &hit_dist)) {
    //x3d_object_reverse_velocity(obj, &obj->velocity, hit_face, hit_dist);
    obj->on_floor = X3D_TRUE;
    
    X3D_Vex3D_fp8x8 new_velocity = obj->velocity;
    
    if(x3d_object_keep_velocity_of_sliding_object(obj, &new_velocity, hit_face)) {
      obj->velocity = new_velocity;
    }
    else {
      obj->velocity.x = 0;
      obj->velocity.y = 0;
      obj->velocity.z = 0;
      return;
    }
    
    int i = 0;
    
    do {
      //x3d_log(X3D_INFO, "Add: %d", ((int32)hit_face->plane.normal.y * (obj->bound_sphere.radius - hit_dist)) / 128);
      //break;
      
      new_pos.x += ((int32)hit_face->plane.normal.x * (obj->bound_sphere.radius - hit_dist)) / 128;
      new_pos.y += ((int32)hit_face->plane.normal.y * (obj->bound_sphere.radius - hit_dist)) / 128;
      new_pos.z += ((int32)hit_face->plane.normal.z * (obj->bound_sphere.radius - hit_dist)) / 128;
    
      sphere.radius = obj->bound_sphere.radius;
      sphere.center = x3d_vex3d_make(
        new_pos.x >> 8,
        new_pos.y >> 8,
        new_pos.z >> 8
      );      
    } while(!x3d_boundsphere_inside_levelsegment(&sphere, segment, &hit_face, &hit_dist) && ++i < 5);
    
    obj->base.pos = new_pos;
  }
  
  //if(abs(obj->bound_sphere.radius - hit_dist) < 20)
  //  x3d_velocity_apply_friction(&obj->velocity);
  
  obj->base.pos = new_pos;
}

