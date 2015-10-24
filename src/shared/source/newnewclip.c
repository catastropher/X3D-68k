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
#include "X3D_segment.h"
#include "X3D_vector.h"
#include "X3D_render.h"
#include "X3D_trig.h"
#include "X3D_clip.h"
#include "X3D_frustum.h"
#include "X3D_engine.h"
#include "X3D_memory.h"
#include "X3D_newclip.h"

typedef struct X3D_RasterEdge {
  union {
    uint8    start_y;
    uint8    start_x;
  };
  
  union {
    uint8    end_y;
    uint8    end_x;
  };
  
  int16*    data;
  uint16    flags;
} X3D_RasterEdge;


enum {
  EDGE_INVISIBLE = 1,
  EDGE_HORIZONTAL = 2,
  EDGE_RASTERED_LEFT = 4,
  EDGE_RASTERED_RIGHT = 8,
  
  EDGE_RASTERED_TOP = 4,
  EDGE_RASTERED_BOTTOM = 8
};

int32 vertical_slope(Vex2D v1, Vex2D v2);

void init_edge(X3D_RasterEdge* edge, Vex2D a, Vex2D b, int16 min_y, int16 max_y) {
  edge->flags = 0;
  
  if((a.y < min_y && b.y < min_y) || (a.y > max_y && b.y > max_y)) {
    // Invisible edge
    edge->flags = EDGE_INVISIBLE;
  }
  else if(a.y == b.y) {
    // Horizontal edge
    edge->flags = EDGE_HORIZONTAL;
    
    if(a.x < b.x) {
      edge->start_x = a.x;
      edge->end_x = b.x;
    }
    else {
      edge->start_x = b.x;
      edge->end_x = a.x;
    }
  }
  else {
    x = x0 + (y1 - y0)m
  }
}








































