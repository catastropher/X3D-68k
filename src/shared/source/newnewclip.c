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
  uint16 flags;
  
  union {
    int16 min_y;
    int16 min_x;
  };
  
  union {
    int16 max_y;
    int16 max_x;
  };
  
  int16* x_data;
} X3D_RasterEdge;

enum {
  EDGE_HORIZONTAL = 1
};


typedef struct X3D_RenderStack {
  uint8* ptr;
  uint8* base;
  uint8* end;
} X3D_RenderStack;

#define SWAP(_a, _b) { typeof(_a) _temp; _temp = _a; _a = _b; _b = _temp; };


void* renderstack_alloc(X3D_RenderStack* stack, uint16 size) {
  stack->ptr -= size + (size & 1);
  
  if(stack->ptr < stack->base) {
    x3d_error("Render stack overflow");
  }
  
  return stack->ptr;
}

void renderstack_init(X3D_RenderStack* stack, uint16 size) {
  stack->base = malloc(size);
  stack->end = stack->base + size;
  stack->ptr = stack->end;
}

void renderstack_cleanup(X3D_RenderStack* stack) {
  free(stack->base);
}

int32 vertical_slope(Vex2D v1, Vex2D v2);

void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, int16 min_y, int16 max_y) {
  if(a.y > b.y) {
    SWAP(a, b);
  }
  
  if(a.y == b.y) {
    // Case 1: edge is horizontal
    edge->flags = EDGE_HORIZONTAL;
    
    if(a.x < b.x) {
      edge->min_x = a.x;
      edge->max_x = b.x;
    }
    else {
      edge->min_x = b.x;
      edge->max_x = a.x;
    }
    
  }
  else {
    fp16x16 x;
    int16 y;
    fp16x16 slope = vertical_slope(a, b);
    
    if(a.y >= min_y) {
      x = ((fp16x16)a.x) << 16;
      y = a.y;
    }
    else {
      fp8x8 new_slope = slope >> 8;
      
      x = (((int32)a.x << 8) + ((int32)(min_y - a.y) * new_slope)) << 8;
      y = min_y;
    }
    
    int16 end_y = min(max_y, b.y);
    
    edge->min_y = y;
    
    edge->flags = 0;
    edge->x_data = renderstack_alloc(stack, (end_y - y + 1) * 2);
    
    while(y <= end_y) {
      edge->x_data[y - edge->min_y] = x >> 16;
      x += slope;
      ++y;
    }
    
    edge->max_y = end_y;
  }
}

void draw_edge(X3D_RasterEdge* edge) {
  int16 y = edge->min_y;
  
  while(y <= edge->max_y) {
    DrawPix(edge->x_data[y - edge->min_x], y, A_NORMAL);
    ++y;
  }
}

void raster_tri(X3D_RasterEdge* edge_left, X3D_RasterEdge* edge_right) {
  int16 y = edge_left->min_y;
  
  while(y <= edge_left->max_y) {
    FastDrawHLine(LCD_MEM, edge_left->x_data[y - edge_left->min_y], edge_right->x_data[y - edge_left->min_y], y, A_NORMAL);
    ++y;
  }
}

void test_newnew_clip() {
  clrscr();
  
  X3D_RenderStack stack;
  renderstack_init(&stack, 1024);
  
  X3D_RasterEdge edge;
  
  Vex2D a = { 128, -200 };
  Vex2D b = { 10, 300 };
  Vex2D c = { 200, 300 };
  
  
  
  generate_rasteredge(&stack, &edge, a, b, 40, 80);
  //draw_edge(&edge);
  
  X3D_RasterEdge edge2;
  generate_rasteredge(&stack, &edge2, a, c, 40, 80);
  
  raster_tri(&edge, &edge2);
  
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 40, A_NORMAL);
  
  
  ngetchx();
}






































