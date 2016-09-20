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

struct X3D_Prism3D;

X3D_LevelSegment* x3d_level_add_new_standalone_segment(X3D_Level* level, struct X3D_Prism3D* seg_geometry, uint16 flags);

void x3d_levelsegment_get_geometry(X3D_Level* level, X3D_LevelSegment* seg, struct X3D_Prism3D* dest);
X3D_LevelSegFace* x3d_levelsegment_get_face_attributes(X3D_Level* level, X3D_LevelSegment* seg);
X3D_LevelSegFace* x3d_levelsegment_get_face_attribute(X3D_Level* level, X3D_LevelSegment* seg, uint16 face);

X3D_LevelSegment* x3d_level_add_uninitialized_segment(X3D_Level* level);
void x3d_levelsegment_initialize_geometry(X3D_Level* level, X3D_LevelSegment* seg, struct X3D_Prism3D* prism);

void x3d_levelsegment_update_plane_normals(X3D_Level* level, X3D_LevelSegment* seg);

X3D_LevelSegment* x3d_level_add_extruded_segment(X3D_Level* level, X3D_SegFaceID seg_to_attach_to, int16 extrude_dist);
void x3d_levelsegment_update_geometry(X3D_Level* level, X3D_LevelSegment* seg, struct X3D_Prism3D* new_geo);

static inline uint16 x3d_level_segment_total_faces(X3D_LevelSegment* seg) {
  return seg->base_v + 2;
}

static inline void x3d_levelsegment_set_flags(X3D_LevelSegment* seg, uint16 flags) {
  seg->flags = flags;
}

void x3d_levelsegment_set_wall_segs_for_face(X3D_Level* level, X3D_LevelSegment* seg, uint16 face, X3D_SegFaceID* wall_segs, uint16 total_wall_segs);

#define X3D_POLYGON3D_ALLOCA_BIG_ENOUGH_TO_HOLD_SEGMENT_LARGEST_FACE(_segmentptr) { .total_v = _segmentptr->base_v, .v = alloca(_segmentptr->base_v * sizeof(X3D_Point3D)) }

#ifdef X3D_LEVEL_SEGMENT_C

#endif

