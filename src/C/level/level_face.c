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

#define X3D_LEVEL_FACE_C

#include "X3D_common.h"
#include "level/X3D_level.h"

X3D_LEVEL_SEG_FACE_RUN x3d_level_segment_face_run_add(X3D_Level* level, X3D_LevelSegFace* run, uint16 total) {
  X3D_LEVEL_SEG_FACE_RUN run_start = level->faces.total;
  
  x3d_level_segment_face_run_expand(level, total);
  
  if(x3d_level_segment_face_run_should_copy_faces(run))
    x3d_level_segment_face_run_copy(level, run_start, run, total);
  else
    x3d_level_segment_face_run_default_init(level, run_start, total);
  
  return run_start;
}

static inline void x3d_level_segment_face_run_expand(X3D_Level* level, uint16 expand_by) {
  level->faces.faces = realloc(level->faces.faces, sizeof(X3D_LevelSegFace) * (level->faces.total + expand_by));
  level->faces.total += expand_by;
}

static inline void x3d_level_segment_face_run_copy(X3D_Level* level, X3D_LEVEL_SEG_FACE_RUN run_start, X3D_LevelSegFace* from, uint16 total) {
  uint16 i;
  for(i = 0; i < total; ++i)
    level->faces.faces[i + run_start] = from[i];
}

static inline _Bool x3d_level_segment_face_run_should_copy_faces(X3D_LevelSegFace* faces) {
  return faces != NULL;
}

static inline void x3d_level_segment_face_run_default_init(X3D_Level* level, X3D_LEVEL_SEG_FACE_RUN run_start, uint16 total) {
  uint16 i;
  for(i = 0; i < total; ++i)
    level->faces.faces[i + run_start].connect_face = X3D_FACE_NONE;
}


