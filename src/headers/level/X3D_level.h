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
#include "X3D_plane.h"

#include "level/X3D_level_types.h"
#include "level/X3D_level_vertex.h"
#include "level/X3D_level_face.h"
#include "level/X3D_level_segment.h"

void x3d_level_init(X3D_Level* level);
void x3d_level_cleanup(X3D_Level* level);

void x3d_test_level();

_Bool x3d_level_save(X3D_Level* level, const char* filename);
_Bool x3d_level_load(X3D_Level* level, const char* filename);


static inline X3D_LevelSegment* x3d_level_expand_segment_array_by_one(X3D_Level* level) {
  X3D_LEVEL_SEG new_seg_id = level->segs.total++;
  level->segs.segs = (X3D_LevelSegment* )realloc(level->segs.segs, sizeof(X3D_LevelSegment) * level->segs.total);
  
  X3D_LevelSegment* new_seg = level->segs.segs + new_seg_id;
  new_seg->id = new_seg_id;
  
  return new_seg;
}

static inline X3D_LevelSegment* x3d_level_get_segmentptr(X3D_Level* level, X3D_LEVEL_SEG id) {
  return level->segs.segs + id;
}

static inline uint16 x3d_level_total_segs(X3D_Level* level) {
    return level->segs.total;
}

X3D_LevelSegment* x3d_level_add_wall_segment_to_center_of_face(X3D_Level* level, X3D_LevelSegment* seg, uint16 face, uint16 base_v, uint16 r, uint16 h);
X3D_LevelSegment* x3d_level_add_segment_connecting_faces(X3D_Level* level, X3D_SegFaceID face_a, X3D_SegFaceID face_b);

