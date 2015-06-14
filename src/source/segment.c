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

typedef struct {

} X3D_EdgeClip;



typedef struct {
  uint8 from_edge, to_edge;
} X3D_ConnectEdge;

typedef struct {
  uint8 total_v;
  
  uint32 edge_bits;     // For edge propogration

  uint16 v[0];           // Vertex ID's that make up the face
  X3D_ConnectEdge connect_edge[0];

} X3D_Face;
  


typedef struct {
  uint8 total_e;        // Total number of edges
  uint8 total_v;         // Total number of veritices
  uint8 total_f;        // Total number of faces
  
  uint16 engine_delta;  // Last delta the segment was rendered
  uint32 drawn_edges;    // Which edges have already been drawn
  
  X3D_EdgeClip* clipped_edges[];
} X3D_Segment;



// Constructs a prism with regular polygons as the base
/// @todo document
void x3d_prism_construct(X3D_Prism* s, uint16 steps, uint16 r, int16 h, X3D_Vex3D_angle256 rot_angle) {
  ufp8x8 angle = 0;
  ufp8x8 angle_step = 65536L / steps;
  uint16 i;

  for(i = 0; i < steps; ++i) {
    s->v[i].x = mul_fp0x16_by_int16_as_int16(x3d_cosfp(uint16_upper(angle)), r);
    s->v[i].z = mul_fp0x16_by_int16_as_int16(x3d_sinfp(uint16_upper(angle)), r);
    s->v[i].y = -h / 2;

    angle += angle_step;
  }

  for(i = 0; i < steps; ++i) {
    s->v[i + steps].x = s->v[i].x;
    s->v[i + steps].z = s->v[i].z;
    s->v[i + steps].y = h / 2;
  }

  // Rotate the prism
  X3D_Mat3x3_fp0x16 mat;
  x3d_mat3x3_fp0x16_construct(&mat, &rot_angle);

  for(i = 0; i < steps * 2; ++i) {
    X3D_Vex3D_int16 rot;
    x3d_vex3d_int16_rotate(&rot, &s->v[i], &mat);
    s->v[i] = rot;

    s->v[i].z += 200;
  }

  s->base_v = steps;
}

void x3d_prism_render(X3D_Prism* prism, X3D_RenderContext* context) {
  uint16 i, d;

  //printf("Steps: %d\n", prism->base_v);
  //ngetchx();


  X3D_Vex2D_int16 screen[prism->base_v * 2];

  for(i = 0; i < prism->base_v * 2; ++i) {
    x3d_vex3d_int16_project(&screen[i], &prism->v[i], context);
  }

  for(i = 0; i < 2; ++i) {
    uint16 start = prism->base_v * i;

    for(d = 0; d < prism->base_v; ++d) {
      uint16 v = start + d;
      uint16 next = start + ((start + d + 1) % prism->base_v);

      x3d_draw_line_black(context, screen[v], screen[next]);
    }
  }

  for(i = 0; i < prism->base_v; ++i) {
    x3d_draw_line_black(context, screen[i], screen[i + prism->base_v]);
  }
}

