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

typedef struct X3D_PlaneCollision {
  X3D_SegmentFace* face;
  int16 dist;
} X3D_PlaneCollision;

_Bool x3d_point_in_segment(X3D_Segment* seg, Vex3D* p, int16 radius, X3D_PlaneCollision* pc) {
  uint16 i;

  X3D_SegmentFace* face = x3d_segment_get_face(seg);

  for(i = 0; i < x3d_segment_total_f(seg); ++i) {
    //printf("dist: %d\n", x3d_distance_to_plane(&face[i].plane, p));
    int16 dist = x3d_distance_to_plane(&face[i].plane, p);

    if(dist < radius) {
      pc->dist = dist;
      pc->face = &face[i];
      return FALSE;
    }
  }

  return TRUE;
}

#define X3D_MAX_COLLISION_ATTEMPS 5

static inline x3d_add_seg_pos(uint16 seg_list[], uint16* seg_list_size, uint16 seg) {

}

static inline _Bool x3d_attempt_adjust_inside_segment(X3D_Segment* seg, Vex3D_fp16x16* pos) {
  X3D_PlaneCollision pc;
  uint16 attempt;
  Vex3D new_pos;

  for(attempt = 0; attempt < X3D_MAX_COLLISION_ATTEMPS; ++attempt) {
    vex3d_fp16x16_to_vex3d(pos, &new_pos);

    _Bool inside = x3d_point_in_segment(seg, &new_pos, 10, &pc);

    if(inside) {
      return TRUE;
    }

    pc.dist -= 10;
    pc.dist = -pc.dist;

    Vex3D_int32 shift = {
      (int32)pc.face->plane.normal.x * pc.dist,
      (int32)pc.face->plane.normal.y * pc.dist,
      (int32)pc.face->plane.normal.z * pc.dist
    };

    *pos = V3ADD(pos, &shift);
  }

  return FALSE;
}

_Bool x3d_attempt_move_object(X3D_Context* context, void* object, Vex3D_fp16x16* dir) {
  X3D_Object* obj = (X3D_Object*)object;
  Vex3D_fp16x16 new_pos_fp16x16 = V3ADD(&obj->pos, dir);
  Vex3D new_pos;
  X3D_PlaneCollision pc;

  uint16 attempt = 0;

  uint16 seg_id = 0;

  uint16 new_seg_list[X3D_MAX_OBJECT_SEGS];
  uint16 new_seg_list_size = 0;

  uint16 seg_pos;
  uint16 i;


  for(seg_pos = 0; seg_pos < X3D_MAX_OBJECT_SEGS; ++seg_pos) {
    if(obj->seg_pos.segs[seg_pos] != SEGMENT_NONE) {
      vex3d_fp16x16_to_vex3d(&new_pos_fp16x16, &new_pos);

      X3D_Segment* seg = x3d_get_segment(context, obj->seg_pos.segs[seg_pos]);

      _Bool inside = x3d_point_in_segment(seg, &new_pos, 10, &pc);

      // Are we close enough to this segment to still be considered inside of it?
      if(abs(pc.dist) <= 10 || inside) {
        x3d_add_seg_pos(new_seg_list, &new_seg_list_size, seg->id);
      }

      if(!inside) {
        // Try moving the object along the normal of the plane it failed against
        if(pc.face->connect_id == SEGMENT_NONE) {
          if(!x3d_attempt_adjust_inside_segment(seg, &new_pos_fp16x16)) {
            printf("Non!\n");
            return FALSE;
          }
          else {
            printf("Repositioned\n");
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
            if(x3d_attempt_adjust_inside_segment(seg, &new_pos_fp16x16)) {
              x3d_add_seg_pos(new_seg_list, &new_seg_list_size, pc.face->connect_id);
            }
            else {
              // If it couldn't be adjusted, it's impossible to slide along the wall
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
}

