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

void x3d_attempt_move_object(X3D_Context* context, void* object, Vex3D_int32* dir) {
  X3D_Object* obj = (X3D_Object*)object;
  Vex3D_fp16x16 new_pos_fp16x16 = V3ADD(&obj->pos, dir);
  Vex3D new_pos;
  X3D_PlaneCollision pc;

  uint16 attempt = 0;

  uint16 seg_id = 0;

  X3D_Segment* seg = x3d_get_segment(context, seg_id);

  for(attempt = 0; attempt < X3D_MAX_COLLISION_ATTEMPS; ++attempt) {
    vex3d_fp16x16_to_vex3d(&new_pos_fp16x16, &new_pos);

    if(x3d_point_in_segment(seg, &new_pos, 10, &pc)) {
      obj->pos = new_pos_fp16x16;
      return;
    }
	
	// Try moving the object along the normal of the plane it failed against

    pc.dist -= 10;

    pc.dist = -pc.dist;

    Vex3D_int32 shift = {
      (int32)pc.face->plane.normal.x * pc.dist,
      (int32)pc.face->plane.normal.y * pc.dist,
      (int32)pc.face->plane.normal.z * pc.dist
    };

    new_pos_fp16x16 = V3ADD(&new_pos_fp16x16, &shift);
  }
}

