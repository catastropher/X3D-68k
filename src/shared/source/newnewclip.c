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
  uint8 flags;
  
  union {
    int16 min_y;    
  };
  
  union {
    int16 max_y;
    int16 max_x;
  };
  
  union {
    int16* x_data;
    int16 min_x;
  };
} X3D_RasterEdge;

enum {
  EDGE_HORIZONTAL = 1,
  EDGE_INVISIBLE = 2
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
    x3d_error("Render stack overflow (need %u)", size);
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

void draw_edge(X3D_RasterEdge* edge) {
  int16 y = edge->min_y;
  
  while(y <= edge->max_y) {
    DrawPix(edge->x_data[y - edge->min_y], y, A_NORMAL);
    ++y;
  }
}

void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, int16 min_y, int16 max_y) {
  if(a.y > b.y) {
    SWAP(a, b);
  }
  
  if(b.y < min_y || a.y > max_y) {
    edge->flags = EDGE_INVISIBLE;
  }
  else if(a.y == b.y) {
    printf("horizontal!\n");
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
    
    edge->min_y = a.y;
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

typedef struct X3D_RasterRegion {
  int16 min_y;
  int16 max_y;
  
  int16* x_left;
  int16* x_right;
} X3D_RasterRegion;

uint16 find_top_edge(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e) {
  int16 min_index = 0;
  uint16 i;
  
  for(i = 1; i < total_e; ++i) {
    uint16 index = edge_index[i];
    X3D_RasterEdge* edge = raster_edge + index;
    X3D_RasterEdge* min_edge = raster_edge + min_index;
    
    if(edge->min_y < min_edge->min_y) {
      min_index = i;
    }
    else if(edge->min_y == min_edge->min_y) {
      // Only update the edge if it's not horizontal and it's more left than the current one
      if(!(min_edge->flags & EDGE_HORIZONTAL) && edge->x_data[1] < min_edge->x_data[1]) {
        min_index = i;
      }
    }
  }
  
  return min_index;
}

uint16 find_bottom_edge(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e) {
  int16 max_index = 0;
  uint16 i;
  
  for(i = 1; i < total_e; ++i) {
    uint16 index = edge_index[i];
    X3D_RasterEdge* edge = raster_edge + index;
    X3D_RasterEdge* max_edge = raster_edge + max_index;
    
    if(edge->max_y > max_edge->max_y) {
      max_index = i;
    }
    else if(edge->max_y == max_edge->max_y) {
      // Only update the edge if it's not horizontal and it's more left than the current one
      if(!(max_edge->flags & EDGE_HORIZONTAL) && edge->x_data[1] < max_edge->x_data[1]) {
        max_index = i;
      }
    }
  }
  
  return max_index;
}

uint16 next_edge(uint16 edge, uint16 total_e, int16 dir) {
  if(dir < 0) {
    return (edge == 0 ? total_e - 1 : edge - 1);
  }
  else {
    return (edge == total_e - 1 ? 0 : edge + 1);
  }
}

int16* populate_edge(X3D_RasterEdge* edge, int16* dest, _Bool last_edge, _Bool left) {
  if(edge->flags & EDGE_HORIZONTAL) {
    *dest = left ? edge->min_x : edge->max_x;
    
    return dest + 1;
  }
  else {
    int16 count = edge->max_y - edge->min_y + (last_edge ? 1 : 0);
    int16* data = edge->x_data;
    
    while(count-- > 0) {
      *dest = *data;
      ++dest;
      ++data;
    }
    
    return dest;
  }
}

int16 populate_polyline(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, uint16 start_edge,
                        int16 dir, uint16 end_edge, int16* dest, _Bool left) {
  uint16 index = start_edge;
  int16* start = dest;
  
  if(index == end_edge) {
    X3D_RasterEdge* edge = raster_edge + edge_index[index];
    
    if(edge->flags & EDGE_INVISIBLE){
      return 0;
    }
    else {
      dest = populate_edge(edge, dest, 1, left);
    }
  }
  else {
    uint16 next;
    _Bool next_invisible = FALSE;
    
    int16 end = next_edge(end_edge, total_e, dir);
    
    while((raster_edge[edge_index[index]].flags & EDGE_INVISIBLE) && index != end) {
      ++index;
    }
    
    if(index == end)
      return 0;
    
    do {
      next = next_edge(index, total_e, dir);
      next_invisible = raster_edge[edge_index[next]].flags & EDGE_INVISIBLE;
      
      dest = populate_edge(raster_edge + edge_index[index], dest, next == end || next_invisible, left);
      index = next;
    } while(next != end && !next_invisible);
  }
  
  return dest - start;
}

void intersect_rasterregion(X3D_RasterRegion* portal, X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, X3D_RasterRegion* dest) {
  uint16 top_index = find_top_edge(raster_edge, edge_index, total_e);
  
  X3D_RasterEdge* top = raster_edge + top_index;
  
  //printf("Top index: %d\n", top_index);
  
  uint16 bottom_index = find_bottom_edge(raster_edge, edge_index, total_e);
  
  //printf("Bottom index: %d\n", bottom_index);
  
  int16 left_dir;
  
  X3D_RasterEdge* next_left = raster_edge + next_edge(top_index, total_e, -1);
  X3D_RasterEdge* next_right = raster_edge + next_edge(top_index, total_e, 1);
  
  if(next_left->min_y == top->min_y) {
    left_dir = 1;
  }
  else {
    left_dir = -1;
  }
  
  int16 left[128];
  int16 right[128];
  
  int16 start_right;
  
  if(top->flags & EDGE_HORIZONTAL) {
    start_right = top_index;
  }
  else {
    start_right = next_edge(top_index, total_e, -left_dir);
  }
  
  int16 total_left = populate_polyline(raster_edge, edge_index, total_e, top_index, left_dir, bottom_index, left, TRUE);
  
  int16 total_right = populate_polyline(raster_edge, edge_index, total_e, start_right, -left_dir, bottom_index, right, FALSE);
  
  if(total_left != total_right) {
    printf("ERROR\n");
  }
  
  int16 i;
  
  for(i = 0; i < total_left; ++i) {
    //DrawPix(left[i], i + top->min_y, A_NORMAL);
    //DrawPix(right[i], i + top->min_y, A_NORMAL);
    
    FastDrawHLine(LCD_MEM, left[i], right[i], i + top->min_y, A_NORMAL);
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
  renderstack_init(&stack, 2048);
  
  X3D_RasterEdge edge;
  
  Vex2D v[] = {
    { 200, 110 },
    { 90, 50 },
    { 20, 30 },
    { 120, 127 }
  };
  
  
  uint16 i;
  
  uint16 TOTAL = 4;
  
  for(i = 0; i < TOTAL; ++i) {
    int16 next = (i + 1) % TOTAL;
    //DrawLine(v[i].x, v[i].y, v[next].x, v[next].y, A_NORMAL);
  }
  
  X3D_RasterEdge edges[20];
  
  for(i = 0; i < TOTAL; ++i) {
    int16 next = (i + 1) % TOTAL;
    generate_rasteredge(&stack, edges + i, v[i], v[next], 0, LCD_HEIGHT - 1);
    
    draw_edge(edges + i);
    ngetchx();
  }
  
  uint16 edge_index[] =  { 0, 1, 2, 3 };
  
  intersect_rasterregion(NULL, edges, edge_index, 4, NULL);
  
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 50, A_NORMAL);
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 60, A_NORMAL);

  ngetchx();
  clrscr();
  
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 50, A_NORMAL);
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 60, A_NORMAL);
  
  for(i = 0; i < TOTAL; ++i) {
    int16 next = (i + 1) % TOTAL;
    generate_rasteredge(&stack, edges + i, v[i], v[next], 50, 60);
    
    //draw_edge(edges);
    //ngetchx();
  }
  
  intersect_rasterregion(NULL, edges, edge_index, 4, NULL);
  
  
  
  
  
#if 0
  
  generate_rasteredge(&stack, &edge, a, b, 40, 80);
  //draw_edge(&edge);
  
  X3D_RasterEdge edge2;
  generate_rasteredge(&stack, &edge2, a, c, 40, 80);
  
  raster_tri(&edge, &edge2);
  
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 40, A_NORMAL);
#endif
  
  
  ngetchx();
}






































