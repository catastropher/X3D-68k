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

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_segment.h"
#include "X3D_trig.h"
#include "X3D_render.h"
#include "X3D_matrix.h"
#include "X3D_geo.h"
#include "X3D_engine.h"
#include "X3D_geo.h"
#include "X3D_object.h"

/**
* Constructs a prism with regular polygons as the base.
*
* @param s          - pointer to the dest prism
* @param steps      - number of "steps" i.e. points on the polygon base
* @param r          - radius of the base
* @param h          - height of the rism
* @param rot_angle  - angles of rotation around its origin
*
* @return nothing
* @note @ref X3D_Prism is a variable-sized data structure. Make sure s is
*     at least sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * steps * 2 bytes big!
*/
void x3d_prism_construct(X3D_Prism* s, uint16 steps, uint16 r, int16 h, Vex3D_angle256 rot_angle) {
  ufp8x8 angle = 0;
  ufp8x8 angle_step = 65536L / steps;
  uint16 i;

  s->base_v = steps;

  // Construct the two bases (regular polygons)
  for(i = 0; i < steps; ++i) {
    s->v[i].x = mul_fp0x16_by_int16_as_int16(x3d_cosfp(uint16_upper(angle)), r);
    s->v[i].z = mul_fp0x16_by_int16_as_int16(x3d_sinfp(uint16_upper(angle)), r);
    s->v[i].y = -h / 2;

    uint16 opp = x3d_opposite_vertex(s, i);

    s->v[opp].x = s->v[i].x;
    s->v[opp].z = s->v[i].z;
    s->v[opp].y = h / 2;

    angle += angle_step;
  }

  // Rotate the prism around its center
  X3D_Mat3x3_fp0x16 mat;
  x3d_mat3x3_fp0x16_construct(&mat, &rot_angle);

  for(i = 0; i < steps * 2; ++i) {
    X3D_Vex3D_int16 rot;

    x3d_vex3d_int16_rotate(&rot, &s->v[i], &mat);
    s->v[i] = rot;
  }

  s->draw_edges = 0xFFFFFFFF;
}

/**
* Renders a prism in wireframe.
*
* @param prism - pointer to the prism
* @param context - render context to render to
*
* @return nothing
* @note No clipping is performed, nor is the position of the camera taken into
*     account when rendering.
* @todo Add clipping and take into position of the camera.
*/
void x3d_prism_render(const X3D_Prism* prism, X3D_ViewPort* context) {
  uint16 i, d;
  X3D_Vex2D_int16 screen[prism->base_v * 2];

  // Project all of the points on the screen
  for(i = prism->base_v * 2; (i--);) {
    x3d_vex3d_int16_project(screen + i, prism->v + i, context);
  }

  uint32 edges = prism->draw_edges;

  // Draw the bases
  X3D_Vex2D_int16* base = screen;

  for(i = 0; i < 2; ++i) {
    x3d_draw_line_black(context, base, base + prism->base_v - 1);

    for(d = 0; d < prism->base_v - 1; ++d) {
      if(edges & 1) {
        x3d_draw_line_black(context, base, base + 1);
      }

      ++base;
      edges >>= 1;
    }

    ++base;
  }

  X3D_Vex2D_int16* base_a = screen;
  X3D_Vex2D_int16* base_b = screen + prism->base_v;

  // Draw the connecting lines between the bases
  for(i = 0; i < prism->base_v; ++i) {
    if(edges & 1) {
      x3d_draw_line_black(context, base_a + i, base_b + i);
    }

    edges >>= 1;
  }
}

// Creates a new segment
/// @todo document properly
X3D_Segment* x3d_segment_add(X3D_Context* state, uint16 base_v) {
  X3D_STACK_TRACE;
  
  X3D_Segment* s = (X3D_Segment* )x3d_stack_alloc(&state->segment_manager.segment_data, x3d_segment_needed_size(base_v));

  s->base_v = base_v;

  s->id = x3d_list_uint16_add(&state->segment_manager.segment_offset, (uint8* )s - state->segment_manager.segment_data.base);

  // Initialize all of the faces to not be connected to anything
  s->face_offset = offsetof(X3D_Segment, prism) + sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * base_v * 2;

  uint16 i;
  X3D_SegmentFace* f = x3d_segment_get_face(s);

  for(i = 0; i < x3d_segment_total_f(s); ++i) {
    f[i].connect_id = SEGMENT_NONE;
  }
  
  for(i = 0; i < X3D_MAX_OBJECTS_IN_SEGMENT; ++i) {
    s->objects[i] = X3D_OBJECT_NONE;
  }

  return s;
}

/**
* Returns a pointer to a segment given its id.
* @param state  - engine state
* @param id     - id of the segment
*/
inline X3D_Segment* x3d_get_segment(X3D_Context* context, int id) {
  return (X3D_Segment*)(context->segment_manager.segment_data.base + context->segment_manager.segment_offset.base[id]);
}

// Returns the total number of segments in an enginestate
uint16 x3d_get_total_segments(X3D_Context* context) {
  return context->segment_manager.segment_offset.size;
}

// Calculates the plane equations for every face in a Segment
void x3d_calculate_segment_normals(X3D_Segment* s) {
  uint16 i;

  // Create a polygon that big enough to hold the segment's largest face
  X3D_Polygon3D* poly = ALLOCA_POLYGON3D(s->base_v);
  X3D_SegmentFace* face = x3d_segment_get_face(s);

  Vex3D center;
  
  x3d_prism3d_get_center(&s->prism, &center);
  
  for(i = 0; i < x3d_segment_total_f(s); ++i) {
    x3d_prism3d_get_face(poly, &s->prism, i);
    x3d_plane_construct(&face[i].plane, &poly->v[0], &poly->v[1], &poly->v[2]);
    
    if(x3d_distance_to_plane(&face[i].plane, &center) < 0) {
      face[i].plane.normal.x = -face[i].plane.normal.x;
      face[i].plane.normal.y = -face[i].plane.normal.y;
      face[i].plane.normal.z = -face[i].plane.normal.z;
      face[i].plane.d = -face[i].plane.d;
    }
  }
}

void x3d_add_object_to_segment(X3D_Segment* seg, uint16 obj) {
  uint16 i;
  
  int16 pos = -1;

  for(i = 0; i < X3D_MAX_OBJECTS_IN_SEGMENT; ++i) {
    if(seg->objects[i] == X3D_OBJECT_NONE) {
      pos = i;
    }
    else if(seg->objects[i] == obj)
      return;
  }

  if(pos != -1)
    seg->objects[pos] = obj;
}

void x3d_remove_object_from_segment(X3D_Segment* seg, uint16 obj) {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_OBJECTS_IN_SEGMENT; ++i) {
    if(seg->objects[i] == obj) {
      seg->objects[i] = X3D_OBJECT_NONE;
      break;
    }
  }
}



















