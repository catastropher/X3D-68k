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


typedef struct {
  uint16 base_v;     // Number of vertices in a base
  X3D_Vex3D_int16 v[0];
} X3D_Prism;

struct X3D_RenderContext;

void x3d_prism_construct(X3D_Prism* s, uint16 steps, uint16 r, int16 h, X3D_Vex3D_angle256 rot_angle);
void x3d_prism_render(X3D_Prism* prism, struct X3D_RenderContext* context);

#if 0

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

#endif

