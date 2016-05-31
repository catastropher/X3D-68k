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

#define X3D_LEVEL_VERTEX_C

#include "level/X3D_level.h"

X3D_LEVEL_VERTEX x3d_level_vertex_find(X3D_Level* level, X3D_Vex3D* v) {
  uint16 i;
  for(i = 0; i < level->v.total; ++i)
    if(x3d_vex3d_equal(level->v.v + i, v))
      return i;
  
  return i;
}

X3D_LEVEL_VERTEX x3d_level_vertex_add(X3D_Level* level, X3D_Vex3D* v) {
  uint16 vertex_index = x3d_level_vertex_find(level, v);  
  if(x3d_level_vertex_is_valid_index(level, vertex_index))
    return vertex_index;
  
  x3d_level_vertex_array_expand(level);
  x3d_level_vertex_set(level, vertex_index, v);
  return vertex_index;
}

static inline void x3d_level_vertex_array_expand(X3D_Level* level) {
  level->v.v = realloc(level->v.v, sizeof(X3D_Vex3D) * (level->v.total + 1));
}

X3D_LEVEL_VERTEX_RUN x3d_level_vertex_run_add(X3D_Level* level, X3D_LEVEL_VERTEX* run, uint16 total) {
  uint16 run_start = level->runs.total;
  
  x3d_level_vertex_run_expand(level, total);
  x3d_level_vertex_run_copy(level, run_start, run, total);
  
  return run_start;
}

static inline void x3d_level_vertex_run_expand(X3D_Level* level, uint16 expand_by) {
  level->runs.v = realloc(level->runs.v, sizeof(X3D_LEVEL_VERTEX_RUN) * (level->runs.total + expand_by));
  level->runs.total += expand_by;
}

static inline void x3d_level_vertex_run_copy(X3D_Level* level, X3D_LEVEL_VERTEX_RUN run_start, X3D_LEVEL_VERTEX* from, uint16 total) {
  uint16 i;
  for(i = 0; i < total; ++i)
    level->runs.v[i + run_start] = from[i];
}

