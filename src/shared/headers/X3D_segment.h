/* This file is part of X3D.
*
* X3D is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* X3D is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with X3D. If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#include "X3D_vector.h"
#include "X3D_plane.h"
#include "X3D_polygon.h"

#define SEGMENT_NONE 0xFFFF

typedef struct X3D_Prism3D {
  uint32 draw_edges;        ///< Bitfield of which edges should be drawn
  uint16 base_v;            ///< Number of vertices in each base
  Vex3D_int16 v[0];     ///< Vertices (variable number)
} X3D_Prism3D;

typedef struct X3D_Segment {
  uint16 base_v;
  uint16 id;
  uint16 last_frame;

  //X3D_BoundSphere bound;

  uint16 face_offset;
  X3D_Prism3D prism;

} X3D_Segment;

typedef struct X3D_SegmentFace {
  uint16 connect_id;
  X3D_Plane plane;
} X3D_SegmentFace;

/// @todo document
/// @todo rename to X3D_RenderStack
typedef struct {
  uint16 size;
  uint8* ptr;
  uint8* base;
} X3D_Stack;

/// @todo document
typedef struct X3D_List_uint16 {
  uint16* base;
  uint16 size;
  uint16 capacity;
} X3D_List_uint16;

typedef struct X3D_SegmentManager {
  X3D_List_uint16 segment_offset;
  X3D_Stack segment_data;
} X3D_SegmentManager;

/// @todo document
static inline X3D_SegmentFace* x3d_segment_get_face(X3D_Segment* s) {
  return (X3D_SegmentFace *)((uint8 *)s + s->face_offset);
}

/// @todo document
static inline uint16 x3d_segment_total_v(X3D_Segment* s) {
  return s->base_v * 2;
}

/// @todo document
static inline uint16 x3d_segment_total_f(X3D_Segment* s) {
  return s->base_v + 2;
}

/// @todo document
static inline uint16 x3d_segment_needed_size(uint16 base_v) {
  return sizeof(X3D_Segment) + sizeof(X3D_Prism3D) + sizeof(Vex3D) * base_v * 2 +
    (base_v + 2) * sizeof(X3D_SegmentFace);
}

/// @todo document
static inline uint16 x3d_segment_size(X3D_Segment* s) {
  return sizeof(X3D_Segment) + x3d_segment_total_v(s) * sizeof(Vex3D) +
    x3d_segment_total_f(s) * sizeof(X3D_SegmentFace);
}

static inline uint16 x3d_prism3d_opposite(X3D_Prism3D* prism, uint16 v) {
  return prism->base_v + (prism->base_v - v) - 1;
}

