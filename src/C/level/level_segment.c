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

#define X3D_LEVEL_SEGMENT_C

#include "level/X3D_level.h"
#include "X3D_prism.h"
#include "X3D_segment.h"

X3D_LEVEL_SEG x3d_level_segment_add(X3D_Level* level, X3D_Prism3D* prism, uint16 flags) {
  X3D_LEVEL_SEG seg_index = level->segs.total;
  
  x3d_level_segment_array_expand(level);
  
  X3D_LevelSeg* seg = x3d_level_segment_get(level, seg_index);
  seg->v      = x3d_level_vertex_run_add_from_vex3d_array(level, prism->v, prism->base_v * 2);
  seg->base_v = prism->base_v;
  seg->flags  = flags;
  
  return seg_index;
}

static inline void x3d_level_segment_array_expand(X3D_Level* level) {
  level->segs.segs = realloc(level->segs.segs, sizeof(X3D_LevelSeg) * (level->segs.total + 1));
  ++level->segs.total;
}

X3D_LevelSeg* x3d_level_segment_get(X3D_Level* level, X3D_LEVEL_SEG id) {
  return level->segs.segs + id;
}

uint16 x3d_level_segment_load_v(X3D_Level* level, X3D_LEVEL_SEG id, X3D_Vex3D* v) {
#if 0
  X3D_LevelSeg* seg = x3d_level_segment_get(id);
  
  uint16 i;
  for(i = 0; i < seg->base_v * 2; ++i) {
    v[i] = level->v.v[level->runs[seg->v + i]];
  }
#endif
}

X3D_Segment* x3d_level_segment_load(X3D_Level* level, uint16 id) {
#if 0
  X3D_Segment* seg = malloc(1000);
  X3D_LevelSeg* level_seg = x3d_level_segment_get(id);
  
  seg->base.base_v = level_seg->base_v;
  seg->face_offset = x3d_uncompressedsegment_face_offset(level_seg->base_v);
  
  x3d_level_segment_load_v(level, id, &seg->prism.v);
  
  x3d_segment_reset(seg);
#endif
}

