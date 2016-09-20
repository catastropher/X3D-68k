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

void x3d_levelsegment_connect(X3D_Level* level, X3D_LevelSegment* seg_a, uint16 face_a, X3D_LevelSegment* seg_b, uint16 face_b) {
  x3d_levelsegment_get_face_attribute(level, seg_a, face_a)->connect_face = x3d_segfaceid_create(seg_b->id, face_b);
  x3d_levelsegment_get_face_attribute(level, seg_b, face_b)->connect_face = x3d_segfaceid_create(seg_a->id, face_a);
}

void x3d_levelsegment_initialize_geometry(X3D_Level* level, X3D_LevelSegment* seg, X3D_Prism3D* prism) {
  seg->base_v = prism->base_v;
  seg->v      = x3d_level_vertex_run_add_from_vex3d_array(level, prism->v, prism->base_v * 2);
  seg->faces  = x3d_level_segment_face_run_add(level, NULL, x3d_level_segment_total_faces(seg));
}


void x3d_levelsegment_get_geometry(X3D_Level* level, X3D_LevelSegment* seg, X3D_Prism3D* dest) {
  uint16 i;
  for(i = 0; i < seg->base_v * 2; ++i) {
    dest->v[i] = level->v.v[level->runs.v[seg->v + i]];
  }
  
  dest->base_v = seg->base_v;
}

X3D_LevelSegFace* x3d_levelsegment_get_face_attributes(X3D_Level* level, X3D_LevelSegment* seg) {
  return level->faces.faces + seg->faces;
}

X3D_LevelSegFace* x3d_levelsegment_get_face_attribute(X3D_Level* level, X3D_LevelSegment* seg, uint16 face) {
  return x3d_levelsegment_get_face_attributes(level, seg) + face;
}

void x3d_levelsegment_update_plane_normals(X3D_Level* level, X3D_LevelSegment* seg) {
  X3D_Prism3D level_geo = X3D_ALLOCA_PRISM3D(seg->base_v);
  x3d_levelsegment_get_geometry(level, seg, &level_geo);
  
  X3D_Polygon3D seg_face = X3D_POLYGON3D_ALLOCA_BIG_ENOUGH_TO_HOLD_SEGMENT_LARGEST_FACE(seg);
  X3D_LevelSegFace* seg_face_attributes = x3d_levelsegment_get_face_attributes(level, seg);
  
  uint16 i;
  for(i = 0; i < x3d_level_segment_total_faces(seg); ++i) {
    x3d_prism3d_get_face(&level_geo, i, &seg_face);
    x3d_polygon3d_calculate_plane(&seg_face, &seg_face_attributes[i].plane);    
  }
}


X3D_LevelSegment* x3d_level_add_extruded_segment(X3D_Level* level, X3D_SegFaceID seg_to_attach_to, int16 extrude_dist) {
  X3D_LEVEL_SEG seg_id = x3d_segfaceid_seg(seg_to_attach_to);
  uint16 face_id = x3d_segfaceid_face(seg_to_attach_to);
    
  X3D_LevelSegment* seg = x3d_level_get_segmentptr(level, seg_id);
  
  X3D_Polygon3D seg_face = X3D_POLYGON3D_ALLOCA_BIG_ENOUGH_TO_HOLD_SEGMENT_LARGEST_FACE(seg);  
  X3D_Prism3D seg_geo = X3D_ALLOCA_PRISM3D(seg->base_v);
  x3d_levelsegment_get_geometry(level, seg, &seg_geo);
  
  // Extrude the face along its (flipped) normal
  X3D_Plane plane;
  x3d_prism3d_get_face(&seg_geo, face_id, &seg_face);
  x3d_polygon3d_calculate_plane(&seg_face, &plane);
  x3d_plane_flip(&plane);
  
  X3D_Prism3D new_prism = X3D_ALLOCA_PRISM3D(seg_face.total_v);
  
  x3d_polygon3d_reverse(&seg_face);
  x3d_prism3d_set_face(&new_prism, X3D_BASE_A, &seg_face);
  
  x3d_polygon3d_reverse(&seg_face);
  x3d_polygon3d_translate_normal(&seg_face, &plane.normal, extrude_dist);
  x3d_prism3d_set_face(&new_prism, X3D_BASE_B, &seg_face);
  
  X3D_LevelSegment* new_seg = x3d_level_add_new_standalone_segment(level, &new_prism, 0);
  seg = x3d_level_get_segmentptr(level, seg_id);
  x3d_levelsegment_connect(level, seg, face_id, new_seg, X3D_BASE_A);
  return new_seg;
}

void x3d_levelsegment_update_geometry(X3D_Level* level, X3D_LevelSegment* seg, X3D_Prism3D* new_geo) {
  uint16 i;
  
  x3d_log(X3D_INFO, "base v: %d", seg->base_v);
  
  for(i = 0; i < seg->base_v * 2; ++i) {
    x3d_log(X3D_INFO, "i = %d", i);
    x3d_level_vertex_update(level, level->runs.v[seg->v + i], new_geo->v + i);
  }
  
  /// @todo Only update the segments that share that point
  for(i = 0; i < level->segs.total; ++i) {
    x3d_levelsegment_update_plane_normals(level, x3d_level_get_segmentptr(level, i));
  }
}

void x3d_levelsegment_set_wall_segs_for_face(X3D_Level* level, X3D_LevelSegment* seg, uint16 face, X3D_SegFaceID* wall_segs, uint16 total_wall_segs) {
    uint16 wall_seg_start = level->wall_segs.total;
    
    level->wall_segs.total += total_wall_segs + 1;
    level->wall_segs.wall_segs = realloc(level->wall_segs.wall_segs, level->wall_segs.total * sizeof(X3D_SegFaceID));
    
    level->wall_segs.wall_segs[wall_seg_start] = total_wall_segs;
    
    uint16 i;
    for(i = 0; i < total_wall_segs; ++i) {
        level->wall_segs.wall_segs[i + 1] = wall_segs[i];
    }
    
    x3d_levelsegment_get_face_attribute(level, seg, face)->wall_seg_start = wall_seg_start;
}

