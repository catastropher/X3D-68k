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
#include "X3D_level_types.h"

static inline void x3d_level_vertex_set(X3D_Level* level, X3D_LEVEL_VERTEX index, X3D_Vex3D* v) {
  level->v.v[index] = *v;
}

static inline _Bool x3d_level_vertex_is_valid_index(X3D_Level* level, X3D_LEVEL_VERTEX index) {
  return index < level->v.total;
}

X3D_LEVEL_VERTEX x3d_level_vertex_find(X3D_Level* level, X3D_Vex3D* v);
X3D_LEVEL_VERTEX x3d_level_vertex_add(X3D_Level* level, X3D_Vex3D* v);

X3D_LEVEL_VERTEX_RUN x3d_level_vertex_run_add(X3D_Level* level, X3D_LEVEL_VERTEX* run, uint16 total);
X3D_LEVEL_VERTEX_RUN x3d_level_vertex_run_add_from_vex3d_array(X3D_Level* level, X3D_Vex3D* v, uint16 total_v);

#ifdef X3D_LEVEL_VERTEX_C

static void x3d_level_vertex_array_expand(X3D_Level* level);
static void x3d_level_vertex_run_expand(X3D_Level* level, uint16 expand_by);
static void x3d_level_vertex_run_copy(X3D_Level* level, X3D_LEVEL_VERTEX_RUN run_start, X3D_LEVEL_VERTEX* from, uint16 total);

#endif


