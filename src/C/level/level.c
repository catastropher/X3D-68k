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

void x3d_level_init(X3D_Level* level) {
  level->v.total    = 0;
  level->v.v        = NULL;
  
  level->runs.total = 0;
  level->runs.v     = NULL;
  
  level->segs.total = 0;
  level->segs.segs  = NULL;
  
  level->faces.total = 0;
  level->faces.faces = NULL;
}

void x3d_level_cleanup(X3D_Level* level) {
  free(level->segs.segs);
  free(level->runs.v);
  free(level->v.v);
}

X3D_LevelSegment* x3d_level_add_new_standalone_segment(X3D_Level* level, X3D_Prism3D* seg_geometry, uint16 flags) {  
  X3D_LevelSegment* new_seg = x3d_level_add_uninitialized_segment(level);
  
  x3d_levelsegment_set_flags(new_seg, flags);
  x3d_levelsegment_initialize_geometry(level, new_seg, seg_geometry);
  x3d_levelsegment_update_plane_normals(level, new_seg);
  
  return new_seg;
}

X3D_LevelSegment* x3d_level_add_uninitialized_segment(X3D_Level* level) {
  return x3d_level_expand_segment_array_by_one(level);
}

void x3d_level_test() {
  X3D_Level level;
  x3d_level_init(&level);
  
  X3D_Prism3D* prism = alloca(1000);
  x3d_prism3d_construct(prism, 8, 400, 400, (X3D_Vex3D_angle256) { 0, 0, 0 });
  
  x3d_level_add_new_standalone_segment(&level, prism, 0);
  
  x3d_level_cleanup(&level);
}

static inline void x3d_level_save_vex3d(X3D_Vex3D* v, FILE* file) {
  fprintf(file, "%d %d %d\n", v->x, v->y, v->z);
}

static inline void x3d_level_save_uint16(uint16 val, FILE* file) {
  fprintf(file, "%u\n", val);
}

static inline void x3d_level_save_int16(int16 val, FILE* file) {
  fprintf(file, "%d\n", val);
}

static inline void x3d_level_save_plane(X3D_Plane* plane, FILE* file) {
  x3d_level_save_vex3d(&plane->normal, file);
  x3d_level_save_int16(plane->d, file);
}

static inline void x3d_level_save_vertices(X3D_Level* level, FILE* file) {
  x3d_level_save_uint16(level->v.total, file);
  
  uint16 i;
  for(i = 0; i < level->v.total; ++i)
    x3d_level_save_vex3d(level->v.v + i, file);
}

static inline void x3d_level_save_segment(X3D_LevelSegment* seg, FILE* file) {
  x3d_level_save_uint16(seg->id, file);
  x3d_level_save_uint16(seg->base_v, file);
  x3d_level_save_uint16(seg->v, file);
  x3d_level_save_uint16(seg->faces, file);
  x3d_level_save_uint16(seg->flags, file);
}

static inline void x3d_level_save_segments(X3D_Level* level, FILE* file) {
  x3d_level_save_uint16(level->segs.total, file);
  
  uint16 i;
  for(i = 0; i < level->segs.total; ++i) {
    x3d_level_save_segment(level->segs.segs + i, file);
  }
}

static inline void x3d_level_save_vertex_runs(X3D_Level* level, FILE* file) {
  x3d_level_save_uint16(level->runs.total, file);
  
  uint16 i;
  for(i = 0; i < level->runs.total; ++i)
    x3d_level_save_uint16(level->runs.v[i], file);
}

static inline void x3d_level_save_face_attribute(X3D_LevelSegFace* face, FILE* file) {
  x3d_level_save_uint16(face->connect_face, file);
  x3d_level_save_plane(&face->plane, file);
}

static inline void x3d_level_save_face_attributes(X3D_Level* level, FILE* file) {
  x3d_level_save_uint16(level->faces.total, file);
  
  uint16 i;
  for(i = 0; i < level->faces.total; ++i) {
    x3d_level_save_face_attribute(level->faces.faces + i, file);
  }
}

_Bool x3d_level_save(X3D_Level* level, const char* filename) {
  FILE* file = fopen(filename, "wb");
  
  if(!file)
    return X3D_FALSE;
  
  fprintf(file, "XLEV 1\n");
  x3d_level_save_vertices(level, file);
  x3d_level_save_segments(level, file);
  x3d_level_save_vertex_runs(level, file);
  x3d_level_save_face_attributes(level, file);
  
  fclose(file);
  
  return X3D_TRUE;
}






