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

#include "X3D_engine.h"
#include "X3D_segment.h"
#include "X3D_matrix.h"
#include "X3D_alloc.h"
#include "X3D_trig.h"

// Calculates the plane equations for every face in a Segment
void x3d_calculate_segment_normals(X3D_Segment* s) {
  uint16 i;

  // Create a polygon that big enough to hold the segment's largest face
  X3D_Polygon3D* poly = ALLOCA_POLYGON3D(s->base_v);

  for(i = 0; i < x3d_segment_total_f(s); ++i) {
    //x3d_get_prism3d_face(poly, s);
  }
}

// Creates a new segment
/// @todo document properly
X3D_Segment* x3d_segment_add(X3D_Context* context, uint16 base_v) {

  X3D_Segment* s = (X3D_Segment*)x3d_stack_alloc(&context->segment_manager.segment_data, x3d_segment_needed_size(base_v));

  s->base_v = base_v;

  s->id = x3d_list_uint16_add(&context->segment_manager.segment_offset, (uint8*)s - context->segment_manager.segment_data.base);

  // Initialize all of the faces to not be connected to anything
  s->face_offset = offsetof(X3D_Segment, prism) + sizeof(X3D_Prism3D) + sizeof(Vex3D) * base_v * 2;

  uint16 i;
  X3D_SegmentFace* f = x3d_segment_get_face(s);

  for(i = 0; i < x3d_segment_total_f(s); ++i) {
    f[i].connect_id = SEGMENT_NONE;
  }

  return s;
}

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
void x3d_prism_construct(X3D_Prism3D* s, uint16 steps, uint16 r, int16 h, Vex3D_angle256 rot_angle) {
  ufp8x8 angle = 0;
  ufp8x8 angle_step = 65536L / steps;
  uint16 i;

  s->base_v = steps;

  // Construct the two bases (regular polygons)
  for(i = 0; i < steps; ++i) {
    s->v[i].x = mul_fp0x16_by_int16_as_int16(x3d_cosfp(uint16_upper(angle)), r);
    s->v[i].z = mul_fp0x16_by_int16_as_int16(x3d_sinfp(uint16_upper(angle)), r);
    s->v[i].y = -h / 2;

    uint16 opp = x3d_prism3d_opposite(s, i);

    s->v[opp].x = s->v[i].x;
    s->v[opp].z = s->v[i].z;
    s->v[opp].y = h / 2;

    angle += angle_step;
  }

  // Rotate the prism around its center
  X3D_Mat3x3_fp0x16 mat;
  x3d_mat3x3_fp0x16_construct(&mat, &rot_angle);

  for(i = 0; i < steps * 2; ++i) {
    Vex3D rot;

    x3d_vex3d_int16_rotate(&rot, &s->v[i], &mat);
    s->v[i] = rot;
  }

  s->draw_edges = 0xFFFFFFFF;
}

/**
* Returns a pointer to a segment given its id.
* @param state  - engine state
* @param id     - id of the segment
*/
inline X3D_Segment* x3d_get_segment(X3D_Context* context, int id) {
  return (X3D_Segment*)(context->segment_manager.segment_data.base + context->segment_manager.segment_offset.base[id]);
}

