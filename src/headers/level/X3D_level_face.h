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

#include "level/X3D_level_types.h"

typedef uint16 X3D_SegFaceID;

#define X3D_FACE_NONE 0xFFFF

static inline uint16 x3d_segfaceid_create(uint16 seg, uint16 face) {
  return (seg << 4) + face;
}

static inline uint16 x3d_segfaceid_face(X3D_SegFaceID id) {
  return id & 0x0F;
}

static inline uint16 x3d_segfaceid_seg(X3D_SegFaceID id) {
  return id >> 4;
}

X3D_LEVEL_SEG_FACE_RUN x3d_level_segment_face_run_add(X3D_Level* level, X3D_LevelSegFace* run, uint16 total);

#ifdef X3D_LEVEL_FACE_C

static void x3d_level_segment_face_run_expand(X3D_Level* level, uint16 expand_by);
static void x3d_level_segment_face_run_copy(X3D_Level* level, X3D_LEVEL_SEG_FACE_RUN run_start, X3D_LevelSegFace* from, uint16 total);
static _Bool x3d_level_segment_face_run_should_copy_faces(X3D_LevelSegFace* faces);
static void x3d_level_segment_face_run_default_init(X3D_Level* level, X3D_LEVEL_SEG_FACE_RUN run_start, uint16 total);

#endif
