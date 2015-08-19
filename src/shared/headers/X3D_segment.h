// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_geo.h"

///< Indicates no segment (used to indicate no segment is connected to a face)
#define SEGMENT_NONE 0xFFFF

//=============================================================================
// Forward declarations
//=============================================================================
struct X3D_ViewPort;
struct X3D_EngineState;

//=============================================================================
// Structures
//=============================================================================

typedef struct X3D_BoundSphere {
  Vex3D center;
  int16 radius;
} X3D_BoundSphere;


/// @todo document
typedef struct X3D_Segment {
  uint16 base_v;
  uint16 id;
  uint16 last_frame;
  
  X3D_BoundSphere bound;

  uint16 face_offset;
  X3D_Prism prism;
  
} X3D_Segment;

typedef struct X3D_SegmentFace {
  uint16 connect_id;
  X3D_Plane plane;
} X3D_SegmentFace;


//=============================================================================
// Function declarations
//=============================================================================
void x3d_prism_construct(X3D_Prism* s, uint16 steps, uint16 r, int16 h, Vex3D_angle256 rot_angle);
void x3d_prism_render(const X3D_Prism* prism, struct X3D_ViewPort* context);

X3D_Segment* x3d_segment_add(struct X3D_EngineState* state, uint16 base_v);
X3D_Segment* x3d_get_segment(struct X3D_EngineState* state, int id);

uint16 x3d_get_total_segments(struct X3D_EngineState* state);

//=============================================================================
// Static inline functions
//=============================================================================

/// @todo document
static inline Vex3D* x3d_segment_get_v(X3D_Segment* s) {
  return s->prism.v;
}

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
  return sizeof(X3D_Segment) + sizeof(X3D_Prism) + sizeof(Vex3D) * base_v * 2 +
    (base_v + 2) * sizeof(X3D_SegmentFace);
}

/// @todo document
static inline uint16 x3d_segment_size(X3D_Segment* s) {
  return sizeof(X3D_Segment) + x3d_segment_total_v(s) * sizeof(Vex3D) +
    x3d_segment_total_f(s) * sizeof(X3D_SegmentFace);
}

