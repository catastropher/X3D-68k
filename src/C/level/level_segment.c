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

X3D_LEVEL_SEG x3d_level_segment_add(X3D_Level* level, X3D_Prism3D* prism, uint16 flags) {
  X3D_LEVEL_SEG seg_index = level->segs.total;
  
  x3d_level_segment_array_expand(level);
  
  X3D_LevelSeg* seg = x3d_level_segment_get(level, seg_index);
  seg->base_v = prism->base_v;
  seg->v      = x3d_level_vertex_run_add_from_vex3d_array(level, prism->v, prism->base_v * 2);
  seg->faces  = x3d_level_segment_face_run_add(level, NULL, x3d_level_segment_total_faces(seg));
  seg->flags  = flags;
  seg->id     = seg_index;
  
  x3d_level_segment_update_plane_normals(level, seg);
  
  return seg->id;
}

static inline void x3d_level_segment_array_expand(X3D_Level* level) {
  level->segs.segs = realloc(level->segs.segs, sizeof(X3D_LevelSeg) * (level->segs.total + 1));
  ++level->segs.total;
}

X3D_LevelSeg* x3d_level_segment_get(X3D_Level* level, X3D_LEVEL_SEG id) {
  return level->segs.segs + id;
}

void x3d_level_segment_get_geometry(X3D_Level* level, X3D_LevelSeg* seg, X3D_Prism3D* dest) {
  uint16 i;
  for(i = 0; i < seg->base_v * 2; ++i) {
    dest->v[i] = level->v.v[level->runs.v[seg->v + i]];
  }
  
  dest->base_v = seg->base_v;
}

X3D_LevelSegFace* x3d_level_segment_get_face_attributes(X3D_Level* level, X3D_LevelSeg* seg) {
  return level->faces.faces + seg->faces;
}

X3D_LevelSegFace* x3d_level_segment_get_face_attribute(X3D_Level* level, X3D_LevelSeg* seg, uint16 face) {
  return x3d_level_segment_get_face_attributes(level, seg) + face;
}

void x3d_level_segment_update_plane_normals(X3D_Level* level, X3D_LevelSeg* seg) {
  X3D_Prism3D level_geo = X3D_ALLOCA_PRISM3D(seg->base_v);
  x3d_level_segment_get_geometry(level, seg, &level_geo);
  
  X3D_Polygon3D seg_face = X3D_POLYGON3D_ALLOCA_BIG_ENOUGH_TO_HOLD_SEGMENT_LARGEST_FACE(seg);
  X3D_LevelSegFace* seg_face_attributes = x3d_level_segment_get_face_attributes(level, seg);
  
  uint16 i;
  for(i = 0; i < x3d_level_segment_total_faces(seg); ++i) {
    x3d_prism3d_get_face(&level_geo, i, &seg_face);
    x3d_polygon3d_calculate_plane(&seg_face, &seg_face_attributes[i].plane);
  }
}


X3D_LEVEL_SEG x3d_level_segment_add_extruded_segment(X3D_Level* level, X3D_SegFaceID seg_to_attach_to, int16 extrude_dist) {
  X3D_LEVEL_SEG seg_id = x3d_segfaceid_seg(seg_to_attach_to);
  uint16 face_id = x3d_segfaceid_face(seg_to_attach_to);
    
  X3D_LevelSeg* seg = x3d_level_segment_get(level, seg_id);
  
  X3D_Polygon3D seg_face = X3D_POLYGON3D_ALLOCA_BIG_ENOUGH_TO_HOLD_SEGMENT_LARGEST_FACE(seg);
  
  X3D_Prism3D seg_geo = X3D_ALLOCA_PRISM3D(seg->base_v);
  x3d_level_segment_get_geometry(level, seg, &seg_geo);
  
  // Extrude the face along its (flipped) normal
  X3D_Plane plane;
  x3d_prism3d_get_face(&seg_geo, face_id, &seg_face);
  x3d_polygon3d_calculate_plane(&seg_face, &plane);
  plane.normal = x3d_vex3d_neg(&plane.normal);
  
  X3D_Prism3D new_prism = X3D_ALLOCA_PRISM3D(seg_face.total_v);
  
  // The old faces becomes BASE_A of the new prism. But, if the old face was
  // BASE_B, it needs to be reversed (BASE_B is always reversed after calling
  // x3d_prism3d_get_face()).
  //if(face_id == X3D_BASE_A || face_id == X3D_BASE_B) {
    x3d_polygon3d_reverse(&seg_face);
  //}
  
  x3d_prism3d_set_face(&new_prism, X3D_BASE_A, &seg_face);
  
  // BASE_B of the new prism becomes the translated polygon (but it has to be
  // because x3d_prism3d_set_face() expects BASE_B to be reversed).
  x3d_polygon3d_reverse(&seg_face);
  x3d_polygon3d_translate_normal(&seg_face, &plane.normal, extrude_dist);
  x3d_prism3d_set_face(&new_prism, X3D_BASE_B, &seg_face);
  
  // Create a new segment with the new prism
  X3D_LEVEL_SEG new_seg = x3d_level_segment_add(level, &new_prism, 0);
  
  
  //X3D_Segment* new_seg = x3d_segmentbuilder_add_uncompressed_segment(
  //  new_prism);
  
  //X3D_SegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  //X3D_SegmentFace* new_face = x3d_uncompressedsegment_get_faces(new_seg);
  
  // Add the face portal connecting the two segments
  //face[face_id].portal_seg_face = x3d_segfaceid_create(new_seg->base.id, X3D_BASE_A);
  //new_face[X3D_BASE_A].portal_seg_face = id;
  
  //x3d_segmentmanager_cache_purge();
  
  return new_seg;
}

void x3d_level_segment_update_geometry(X3D_Level* level, X3D_LevelSeg* seg, X3D_Prism3D* new_geo) {
  uint16 i;
  
  x3d_log(X3D_INFO, "base v: %d", seg->base_v);
  
  for(i = 0; i < seg->base_v * 2; ++i) {
    x3d_log(X3D_INFO, "i = %d", i);
    x3d_level_vertex_update(level, level->runs.v[seg->v + i], new_geo->v + i);
  }
  
  /// @todo Only update the segments that share that point
  for(i = 0; i < level->segs.total; ++i) {
    x3d_level_segment_update_plane_normals(level, x3d_level_segment_get(level, i));
  }
}


