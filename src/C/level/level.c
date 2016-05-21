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
#include "level/X3D_level.h"
#include "X3D_prism.h"

X3D_LEVEL_VERTEX x3d_level_vertex_add(X3D_Level* level, X3D_Vex3D* v) {
  uint16 i;
  for(i = 0; i < level->v.total; ++i) {
    if(x3d_vex3d_equal(level->v.v + i, v)) {
      return i;
    }
  }
  
  level->v.v = realloc(level->v.v, sizeof(X3D_Vex3D) * (level->v.total + 1));
  return level->v.total++;
}

X3D_LEVEL_VERTEX_RUN x3d_level_vertex_run_add(X3D_Level* level, X3D_LEVEL_VERTEX* run, uint16 total) {
  uint16 old_total = level->runs.total;
  
  level->runs.v = realloc(level->runs.v, sizeof(X3D_LEVEL_VERTEX_RUN) * (level->runs.total + total));
  level->runs.total += total;
  
  uint16 i;
  for(i = 0; i < total; ++i)
    level->runs.v[i + old_total] = run[i];
  
  return old_total;
}

void x3d_level_init(X3D_Level* level) {
  level->v.total    = 0;
  level->v.v        = NULL;
  
  level->runs.total = 0;
  level->runs.v     = NULL;
  
  level->segs.total = 0;
  level->segs.segs  = NULL;
}

void x3d_test_level() {
  X3D_Level level;
  x3d_level_init(&level);
}

X3D_LEVEL_SEG x3d_level_segment_add(X3D_Level* level, X3D_Prism3D* prism, uint16 flags) {
  // Make room for the new segment
  level->segs.segs = realloc(level->segs.segs, sizeof(X3D_LevelSeg) * (level->segs.total + 1));
  
  X3D_LevelSeg* seg = level->segs.segs + level->segs.total;
  
  X3D_LEVEL_VERTEX v[prism->base_v * 2];
  
  uint16 i;
  for(i = 0; i < prism->base_v * 2; ++i) {
    v[i] = x3d_level_vertex_add(level, prism->v + i);
  }
  
  seg->v = x3d_level_vertex_run_add(level, v, prism->base_v * 2);
  seg->base_v = prism->base_v;
  seg->flags = flags;
  
  ++level->segs.total;
}

