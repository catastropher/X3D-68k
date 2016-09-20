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

#include <stdio.h>

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
  
  level->wall_segs.total = 0;
  level->wall_segs.wall_segs = NULL;
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

X3D_LevelSegment* x3d_level_add_wall_segment_to_center_of_face(X3D_Level* level, X3D_LevelSegment* seg, uint16 face, uint16 base_v, uint16 r, uint16 h) {
    X3D_Prism3D prism = { .v = alloca(1000) };
    X3D_Polygon3D poly3d = { .v = alloca(1000) };
    
    x3d_levelsegment_get_geometry(level, seg, &prism);
    x3d_prism3d_get_face(&prism, face, &poly3d);
    
    X3D_Vex3D center;
    X3D_Plane plane;
    x3d_polygon3d_center(&poly3d, &center);
    x3d_polygon3d_calculate_plane(&poly3d, &plane);
    
    X3D_Polygon2D poly2d = { .v = alloca(1000) };
    x3d_polygon2d_construct(&poly2d, base_v, r, 0);
    X3D_Mat3x3 mat;
    
    x3d_polygon2d_to_polygon3d(&poly2d, &poly3d, &plane, &center, NULL, &mat);
    x3d_polygon3d_reverse(&poly3d);
    plane.normal = x3d_vex3d_neg(&plane.normal);
    x3d_polygon3d_translate(&poly3d, center);
    
    prism.base_v = base_v;
    x3d_prism3d_set_face(&prism, X3D_BASE_A, &poly3d);
    x3d_polygon3d_translate_normal(&poly3d, &plane.normal, h);
    x3d_polygon3d_reverse(&poly3d);
    x3d_prism3d_set_face(&prism, X3D_BASE_B, &poly3d);
    
    X3D_LevelSegment* new_seg = x3d_level_add_new_standalone_segment(level, &prism, 0);
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

//===========
static inline void x3d_level_load_vex3d(X3D_Vex3D* dest, FILE* file) {
  int x, y, z;
  fscanf(file, "%d %d %d\n", &x, &y, &z);
  
  dest->x = x;
  dest->y = y;
  dest->z = z;
}

static inline void x3d_level_load_uint16(uint16* dest, FILE* file) {
  unsigned int val;
  fscanf(file, "%u\n", &val);
  *dest = val;
}

static inline void x3d_level_load_int16(int16* dest, FILE* file) {
  int val;
  fscanf(file, "%u\n", &val);
  *dest = val;
}

static inline void x3d_level_load_plane(X3D_Plane* plane, FILE* file) {
  x3d_level_load_vex3d(&plane->normal, file);
  x3d_level_load_int16(&plane->d, file);
}

static inline void x3d_level_load_vertices(X3D_Level* level, FILE* file) {
  x3d_level_load_uint16(&level->v.total, file);
  
  level->v.v = malloc(sizeof(X3D_Vex3D) * level->v.total);
  
  uint16 i;
  for(i = 0; i < level->v.total; ++i)
    x3d_level_load_vex3d(level->v.v + i, file);
}

static inline void x3d_level_load_segment(X3D_LevelSegment* seg, FILE* file) {
  x3d_level_load_uint16(&seg->id, file);
  x3d_level_load_uint16(&seg->base_v, file);
  x3d_level_load_uint16(&seg->v, file);
  x3d_level_load_uint16(&seg->faces, file);
  x3d_level_load_uint16(&seg->flags, file);
}

static inline void x3d_level_load_segments(X3D_Level* level, FILE* file) {
  x3d_level_load_uint16(&level->segs.total, file);
  
  level->segs.segs = malloc(sizeof(X3D_LevelSegment) * level->segs.total);
  
  uint16 i;
  for(i = 0; i < level->segs.total; ++i) {
    x3d_level_load_segment(level->segs.segs + i, file);
  }
}

static inline void x3d_level_load_vertex_runs(X3D_Level* level, FILE* file) {
  x3d_level_load_uint16(&level->runs.total, file);
  
  level->runs.v = malloc(sizeof(uint16) * level->runs.total);
  
  uint16 i;
  for(i = 0; i < level->runs.total; ++i)
    x3d_level_load_uint16(level->runs.v + i, file);
}

static inline void x3d_level_load_face_attribute(X3D_LevelSegFace* face, FILE* file) {
  x3d_level_load_uint16(&face->connect_face, file);
  x3d_level_load_plane(&face->plane, file);
}

static inline void x3d_level_load_face_attributes(X3D_Level* level, FILE* file) {
  x3d_level_load_uint16(&level->faces.total, file);
  
  level->faces.faces = malloc(sizeof(X3D_LevelSegFace) * level->faces.total);
  
  uint16 i;
  for(i = 0; i < level->faces.total; ++i) {
    x3d_level_load_face_attribute(level->faces.faces + i, file);
  }
}

_Bool x3d_level_load(X3D_Level* level, const char* filename) {
  FILE* file = fopen(filename, "rb");
  
  if(!file)
    return X3D_FALSE;
  
  char level_magic_number[5] = { '\0' };
  int version;
  
  fscanf(file, "%4s %d", level_magic_number, &version);
  
  if(strcmp(level_magic_number, "XLEV") != 0) {
    x3d_log(X3D_ERROR, "Unrecognized level magic number for file '%s'", filename);
    return X3D_FALSE;
  }
  
  if(version != 1) {
    x3d_log(X3D_ERROR, "Unrecognized level format version %d for file '%s'", version, filename);
    return X3D_FALSE;
  }
  
  x3d_level_load_vertices(level, file);
  x3d_level_load_segments(level, file);
  x3d_level_load_vertex_runs(level, file);
  x3d_level_load_face_attributes(level, file);
  
  fclose(file);
  
  return X3D_TRUE;
}




