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

#define SWAP(_a, _b) { typeof(_a) _temp; _temp = _a; _a = _b; _b = _temp; };

void init_edge(X3D_RasterEdge* edge, Vex2D a, Vex2D b, int16 min_y, int16 max_y) {
  edge->flags = 0;
  
  if(a.y > b.y) {
    SWAP(a, b);
  }
  
  if(b.y < min_y || a.x > max_y) {
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
    // Case where both endpoints are visible
    if(a.y >= min_y && b.y <= max_y) {
      int32 slope = vertical_slope(a, b);
      
      int32 x = ((int32)a.x << 16);
      int16 y = a.y;
      
      while(  y <= b.y) {
        edge->data[y - a.y] = x >> 16;
        x += slope;
        ++y;
      }
      
      edge->start_y = a.y;
      edge->end_y = b.y;
      
    }
  }
}

typedef struct X3D_Span_int16 {
  int16 left;
  int16 right;
} X3D_Span_int16;

typedef struct X3D_Span_uint8 {
  uint8 left;
  uint8 right;
} X3D_Span_uint8;

_Bool clip_span(X3D_Span_int16* bound, X3D_Span_int16* span, X3D_Span_int16* dest) {
  if(span->left <= bound->left) {
    dest->left = bound->left + 1;
  }
  else {
    dest->left = span->left;
  }
  
  if(span->right >= bound->right) {
    dest->right = bound->right - 1;
  }
  else {
    dest->right = span->right;
  }
 
  return dest->left <= dest->right;
}

typedef struct X3D_RasterPoly {
  uint16 start_y;
  uint16 end_y;
  
  X3D_Span_int16* spans;
} X3D_RasterPoly;

X3D_Span_int16* get_span(X3D_RasterPoly* poly, uint16 span) {
  return poly->spans + span - poly->start_y;
}

_Bool clip_poly_span(X3D_RasterPoly* bound_poly, X3D_RasterPoly* poly, uint16 y, X3D_Span_int16* span) {
  X3D_Span_int16* bound_span = get_span(bound_poly, y);
  X3D_Span_int16* poly_span = get_span(poly, y);
  
  return clip_span(bound_span, poly_span, span);
}


_Bool intersect_rasterpoly(X3D_RasterPoly* bound, X3D_RasterPoly* poly, X3D_RasterPoly* dest) {
  int16 y = max(bound->start_y, poly->start_y);
  int16 end_y = min(bound->end_y, poly->end_y);
  X3D_Span_int16 span;
  
  // Find the top visible span
  while(y <= end_y && !clip_poly_span(bound, poly, y, &span)) {
    ++y;
  }
  
  // Find the bottom visible span
  while(end_y > y && !clip_poly_span(bound, poly, end_y, &span)) {
    --end_y;
  }
  
  dest->start_y = y;
  dest->end_y = end_y;
  
  while(y <= end_y) {
    clip_poly_span(bound, poly, y, get_span(dest, y));
    ++y;
  }
  
  return y <= end_y;
}




void draw_edge(X3D_RasterEdge* edge) {
  int16 y = edge->start_y;
  
  while(y <= edge->end_y) {
    //printf("%d\n", edge->data[y - edge->start_y]);
    DrawPix(edge->data[y - edge->start_y], y, A_NORMAL);
    ++y;
  }

}

void test_newnew_clip() {
  clrscr();
  
  Vex2D a = { 128, 0 };
  Vex2D b = { 20, 57 };
  
  int16 data[256];
  
  
  X3D_RasterEdge edge;
  edge.data = data;
  init_edge(&edge, a, b, 0, 239);
  draw_edge(&edge);
  
  X3D_RasterEdge edge2;
  
  int16 data2[256];
  edge2.data = data2;
  
  b = (Vex2D){ 200, 57 };
  init_edge(&edge2, a, b, 0, 239);
  draw_edge(&edge2);
  
  uint16 i;
  
  for(i = 0; i < 57; ++i) {
    FastDrawHLine(LCD_MEM, edge.data[i], edge2.data[i], i, A_NORMAL);
  }
  
  
  //printf("Hello from test!\n");
  ngetchx();
}






































