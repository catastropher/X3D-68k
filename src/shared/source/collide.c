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

#include "X3D_config.h"
#include "X3D_frustum.h"
#include "X3D_object.h"
#include "X3D_segment.h"
#include "X3D_vector.h"
#include "X3D_engine.h"

#define FRAC_BITS 2

typedef struct X3D_PlaneCollision {
  X3D_SegmentFace* face;
  int16 dist;
} X3D_PlaneCollision;

_Bool x3d_point_in_segment(X3D_Segment* seg, Vex3D* p, X3D_BoundVolume* volume, X3D_PlaneCollision* pc, _Bool include_portal) {
  uint16 i;

  X3D_SegmentFace* face = x3d_segment_get_face(seg);

  for(i = 0; i < x3d_segment_total_f(seg); ++i) {
    //printf("dist: %d\n", x3d_distance_to_plane(&face[i].plane, p));
    int16 dist = (x3d_vex3d_int16_dot(&face[i].plane.normal, p) >> (X3D_NORMAL_SHIFT - FRAC_BITS)) - ((face[i].plane.d << FRAC_BITS));//x3d_distance_to_plane(&face[i].plane, p);
    int16 radius;

    if(volume->type == X3D_BOUND_CAPSULE) {
      Vex3D top = {
        p->x - volume->capsule.height,
        p->y - volume->capsule.height,
        p->z - volume->capsule.height,
      };

      dist = min(dist, (x3d_vex3d_int16_dot(&face[i].plane.normal, &top) >> (X3D_NORMAL_SHIFT - FRAC_BITS)) - ((face[i].plane.d << FRAC_BITS)));
      radius = volume->capsule.radius;
    }
    else {
      radius = volume->sphere.radius;
    }

    //printf("Dist: %d\n", dist >> FRAC_BITS);

    if((include_portal || face[i].connect_id == SEGMENT_NONE) && (dist >> FRAC_BITS) < radius) {
      pc->dist = dist;
      pc->face = &face[i];
      return FALSE;
    }
  }

  return TRUE;
}

#define X3D_MAX_COLLISION_ATTEMPS 5

static inline x3d_add_seg_pos(uint16 seg_list[], uint16* seg_list_size, uint16 seg) {
  //printf("Added segment %d\n", seg);

  seg_list[(*seg_list_size)++] = seg;
}

static inline _Bool x3d_attempt_adjust_inside_segment(X3D_Segment* seg, Vex3D_fp16x16* pos, _Bool* hit_wall, X3D_BoundVolume* volume) {
  X3D_PlaneCollision pc;
  uint16 attempt;
  Vex3D new_pos;

  *hit_wall = FALSE;
  
  for(attempt = 0; attempt < X3D_MAX_COLLISION_ATTEMPS; ++attempt) {
    vex3d_fp16x16_to_vex3d(pos, &new_pos);

    _Bool inside = x3d_point_in_segment(seg, &new_pos, volume, &pc, FALSE);

    if(inside) {
      return TRUE;
    }
    
    *hit_wall = TRUE;
    
    pc.dist -= (volume->sphere.radius << FRAC_BITS);//volume->sphere.radius;// << FRAC_BITS;
    pc.dist = -pc.dist;

    Vex3D_int32 shift = {
      (int32)pc.face->plane.normal.x * pc.dist,
      (int32)pc.face->plane.normal.y * pc.dist,
      (int32)pc.face->plane.normal.z * pc.dist
    };

    shift.x >>= FRAC_BITS;
    shift.y >>= FRAC_BITS;
    shift.z >>= FRAC_BITS;

    *pos = V3ADD(pos, &shift);
    
    //pos->x = pos->x & ~0x7FFFL;
    //pos->y = pos->y & ~0x7FFFL;
    //pos->z = pos->z & ~0x7FFFL;
  }

  return FALSE;
}

_Bool x3d_attempt_move_object(X3D_Context* context, void* object, Vex3D_fp0x16* dir2, int16 speed2) {
  X3D_Object* obj = (X3D_Object*)object;
  Vex3D_fp16x16 new_pos_fp16x16 = obj->pos;
  Vex3D new_pos;
  X3D_PlaneCollision pc;
  
  Vex3D_fp16x16 old_pos = obj->pos;
  
  Vex3D_fp16x16* dir = &obj->dir;
  int16 speed = 6;//&obj->speed;

  obj->dir.x += obj->gravity.x;
  obj->dir.y += obj->gravity.y;
  obj->dir.z += obj->gravity.z;
  
  
  
  
  new_pos_fp16x16.x = new_pos_fp16x16.x + (int32)dir->x;// * speed;
  new_pos_fp16x16.y = new_pos_fp16x16.y + (int32)dir->y;// * speed;
  new_pos_fp16x16.z = new_pos_fp16x16.z + (int32)dir->z;// * speed;
  
  uint16 attempt = 0;

  uint16 seg_id = 0;

  uint16 new_seg_list[X3D_MAX_OBJECT_SEGS];
  uint16 new_seg_list_size = 0;

  uint16 seg_pos;
  uint16 i;
  
  _Bool hit_wall = FALSE;

  for(i = 0; i < X3D_MAX_OBJECT_SEGS; ++i) {
    new_seg_list[i] = SEGMENT_NONE;
  }


  for(seg_pos = 0; seg_pos < X3D_MAX_OBJECT_SEGS; ++seg_pos) {
    if(obj->seg_pos.segs[seg_pos] != SEGMENT_NONE) {
      vex3d_fp16x16_to_vex3d(&new_pos_fp16x16, &new_pos);

      X3D_Segment* seg = x3d_get_segment(context, obj->seg_pos.segs[seg_pos]);

      _Bool inside = x3d_point_in_segment(seg, &new_pos, &obj->volume, &pc, TRUE);

      // Are we close enough to this segment to still be considered inside of it?
      _Bool found = FALSE;

      for(i = 0; i < X3D_MAX_OBJECT_SEGS; ++i) {
        if(new_seg_list[i] == obj->seg_pos.segs[seg_pos]) {
          found = TRUE;
          break;
        }
      }

      if(!found) {
        if((abs(pc.dist) <= (obj->volume.sphere.radius << FRAC_BITS) || inside)) {
          x3d_add_seg_pos(new_seg_list, &new_seg_list_size, seg->id);
        }
        else {
          // Remove the object from the segment's list of objects
          x3d_remove_object_from_segment(seg, obj->id);
        }
      }

      if(!inside) {
        // Try moving the object along the normal of the plane it failed against
        if(pc.face->connect_id == SEGMENT_NONE) {
          if(!x3d_attempt_adjust_inside_segment(seg, &new_pos_fp16x16, &hit_wall, &obj->volume)) {
            dir->x = 0;
            dir->y = 0;
            dir->z = 0;
            return FALSE;
          }
        }
        else {
          // Alright, so it appears we've moved into another segment
          //if(!x3d_attempt_adjust_inside_segment)

          // There's no point in checking if we moved into the segment on the
          // other side if it's already in the list
          _Bool found = FALSE;

          for(i = 0; i < X3D_MAX_OBJECT_SEGS; ++i) {
            if(new_seg_list[i] == pc.face->connect_id) {
              found = TRUE;
              break;
            }
          }

          if(!found) {
            X3D_Segment* new_seg = x3d_get_segment(context, pc.face->connect_id);

            if(x3d_attempt_adjust_inside_segment(new_seg, &new_pos_fp16x16, &hit_wall, &obj->volume)) {
              x3d_add_seg_pos(new_seg_list, &new_seg_list_size, pc.face->connect_id);
              
              // Add the object to the segment's list of objects
              x3d_add_object_to_segment(new_seg, obj->id);
            }
            else {
              // If it couldn't be adjusted, it's impossible to slide along the wall
              dir->x = 0;
              dir->y = 0;
              dir->z = 0;
              return FALSE;
            }
          }


          //printf("Connect!\n");
        }
      }
    }
  }

  // Yaaay, the object was repositioned successfully! :D
  obj->pos = new_pos_fp16x16;

  for(i = 0; i < X3D_MAX_OBJECT_SEGS; ++i) {
    obj->seg_pos.segs[i] = new_seg_list[i];
  }
  
  if(hit_wall && obj->wall_behavior == X3D_COLLIDE_BOUNCE) {
    
    Vex3D_fp16x16 v = *dir;
    uint16 shift = 0;
    
    while(v.x > 0x7FFF || v.y > 0x7FFF || v.z > 0x7FFF) {
      v.x >>= 1;
      v.y >>= 1;
      v.z >>= 1;
      ++shift;
    }
    
    Vex3D v2 = {v.x, v.y, v.z};
    
    int16 scale = x3d_vex3d_fp0x16_dot(&v2, &pc.face->plane.normal);
    
    obj->dir.x = (((int32)-2 * pc.face->plane.normal.x * scale) >> X3D_NORMAL_SHIFT) + v2.x;
    obj->dir.y = (((int32)-2 * pc.face->plane.normal.y * scale) >> X3D_NORMAL_SHIFT) + v2.y;
    obj->dir.z = (((int32)-2 * pc.face->plane.normal.z * scale) >> X3D_NORMAL_SHIFT) + v2.z;
    
    obj->dir.x <<= shift;
    obj->dir.y <<= shift;
    obj->dir.z <<= shift;
  }
  else if(hit_wall && obj->wall_behavior == X3D_COLLIDE_SLIDE) {
      Vex3D_fp16x16 v = *dir;
    uint16 shift = 0;
    
    
    Vex3D_fp8x8 d = {
      dir->x >> 7,
      dir->y >> 7,      
      dir->z >> 7
    };
    
    
    int16 scale = x3d_vex3d_fp0x16_dot(&d, &pc.face->plane.normal);
    
    
    //scale = ((int32)scale * 128) >> 8;
    
    int16 old = d.y;

    d.x = -(((int32)pc.face->plane.normal.x * scale) >> (X3D_NORMAL_SHIFT)) + d.x;
    d.y = -(((int32)pc.face->plane.normal.y * scale) >> (X3D_NORMAL_SHIFT)) + d.y;
    d.z = -(((int32)pc.face->plane.normal.z * scale) >> (X3D_NORMAL_SHIFT)) + d.z;
    
    sprintf(context->status_bar, "fri %d %d %d", d.x, d.y, d.z);
    
    if(abs(d.x) < 100 && abs(d.y) < 100 && abs(d.z) < 100) {
      dir->x = 0;
      dir->y = 0;
      dir->z = 0;
      
      obj->pos = old_pos;
      
      return TRUE;
    }
    
    Vex3D v2 = d;
    
    x3d_vex3d_fp0x16_normalize(&v2);
    
    Vex3D new_d = {
      d.x - v2.x,
      d.y - v2.y,
      d.z - v2.z
    };
    
    if(X3D_SIGNOF(new_d.x) != X3D_SIGNOF(d.x))
      new_d.x = 0;
    
    if(X3D_SIGNOF(new_d.y) != X3D_SIGNOF(d.y))
      new_d.y = 0;
    
    if(X3D_SIGNOF(new_d.z) != X3D_SIGNOF(d.z))
      new_d.z = 0;
    
    if(abs(new_d.x) < 20 && abs(new_d.y) < 20 && abs(new_d.z) < 20) {
      dir->x = 0;
      dir->y = 0;
      dir->z = 0;
      
      return TRUE;
    }
    
    
    
    
    dir->x = (int32)new_d.x << 7;
    dir->y = (int32)new_d.y << 7;
    dir->z = (int32)new_d.z << 7;
    
  }
  
  return TRUE;
}

