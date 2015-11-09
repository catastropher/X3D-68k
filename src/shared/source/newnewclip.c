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
#include "X3D_newnewclip.h"

#undef printf
#define printf(...) ;

#include "extgraph.h"

#ifndef INT16_MIN
#define INT16_MIN -32767
#endif

#define ASSERT(_s) {if(!(_s)) x3d_error("Assert failed! %s (%s %d)", #_s, __FUNCTION__, __LINE__);}
#define ASSERT_RANGE(_a, _min, _max) ASSERT(_a >= _min && _a <= _max);

#define ASSERT_EQUAL_INT16(_a, _b) { if((_a) != (_b)) x3d_error("Assert failed (line %d)! %s == %s (%d, %d)", __LINE__, #_a, #_b, (int16)(_a), (int16)(_b)); }
#define ASSERT_EQUAL_INT32(_a, _b) { if((_a) != (_b)) x3d_error("Assert failed (line %d)! %s == %s (%ld, %ld)", __LINE__, #_a, #_b, (int32)(_a), (int32)(_b)); }


#ifdef NDEBUG
#undef NDEBUG
#endif

#if 0
#define ENTER() printf("enter %d\n", __LINE__);
#define EXIT() printf("Exit\n");

#else

#define ENTER() ;
#define EXIT() ;

#endif



void* renderstack_save(X3D_RenderStack* stack) {
  return stack->ptr;
}

void renderstack_restore(X3D_RenderStack* stack, void* ptr) {
  stack->ptr = ptr;
}

#define SWAP(_a, _b) { typeof(_a) _temp; _temp = _a; _a = _b; _b = _temp; };

#define ADDRESS(_a) {if((int32)_a & 1) x3d_error("Address: %s\n", #_a);}


void* renderstack_alloc(X3D_RenderStack* stack, uint16 size) {
  
  ADDRESS(stack);
  
  stack->ptr -= size + (size & 1);
  
  ADDRESS((uint8* )stack );
  ADDRESS(stack->ptr);
  ADDRESS((uint8* )stack->base);
  
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

void intersect_line_with_horizontal(fp16x16 slope, Vex2D* start, int16 y) {
  ASSERT((slope >> 16) < 128);    // To prevent overflow when converting to fp8x8 for the slope
  
  int16 dy = y - start->y;
  int16 slope_8x8 = slope >> 8;
  
  start->x = start->x + (((int32)dy * slope_8x8) >> 8);
  start->y = y;
}

_Bool clip_rasteredge(X3D_RasterEdge* edge, Vex2D* a, Vex2D* b, fp16x16* slope, X3D_Range region_y_range) {
  edge->flags = 0;
  edge->x_data = NULL;
  
  edge->start_x = a->x;
  edge->end_x = b->x;
  
  // Swap points if out of order vertically
  if(a->y > b->y) {
    SWAP(*a, *b);
    edge->flags |= EDGE_V_SWAPPED;
  }
  
  edge->y_range = get_range(a->y, b->y);
  
  if(edge->y_range.min == edge->y_range.max) {
    edge->flags |= EDGE_HORIZONTAL;
    
    if(a->x > b->x) {
      edge->flags |= EDGE_V_SWAPPED;
    }
  }
  
  if(!range_overlap(edge->y_range, region_y_range)) {
    printf("Invisible!\n");
    edge->flags |= EDGE_INVISIBLE;
    
    return FALSE;
  }
  
  if(!(edge->flags & EDGE_HORIZONTAL)) {         // Only clip visible and non-horizontal edges
    *slope = vertical_slope(*a, *b);
    
    if(a->y < region_y_range.min) {
      intersect_line_with_horizontal(*slope, a, region_y_range.min);
      edge->y_range.min = region_y_range.min;
    }
    
    // Clamp the max y
    edge->y_range.max = b->y = min(edge->y_range.max, region_y_range.max);
  }
  
  return TRUE;
}

#define EDGE_VALUE(_edge, _y) ((_edge)->x_data[_y - (_edge)->y_range.min])
 
void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, X3D_Range region_y_range) {
  fp16x16 slope;
  
  //ASSERT(region_y_range.min >= 0 && region_y_range.max < LCD_HEIGHT);
  
  if(!(region_y_range.min >= 0 && region_y_range.max < LCD_HEIGHT)) {
    x3d_error("range min: %d, %d", region_y_range.min, region_y_range.max);
  }
  
  //printf("a: %d, b: %d\n", a.x, b.x);
  
  if(clip_rasteredge(edge, &a, &b, &slope, region_y_range)) {     // Only generate the edge if it's (potentially) visible
    fp16x16 x = ((int32)a.x) << 16;
    int16 y = a.y;
    int16 height = b.y - a.y + 1;
    
    ASSERT(region_y_range.min <= region_y_range.max);
    ASSERT(in_range(SCREEN_Y_RANGE, height - 1));
    ASSERT(in_range(region_y_range, a.y));
    ASSERT(in_range(region_y_range, b.y));
    
    // Allocate space for the values
    edge->x_data = renderstack_alloc(stack, height * 2);
    
    while(y <= b.y) {
      EDGE_VALUE(edge, y) = x >> 16;
      x += slope;
      ++y;
    }
    
    b.x = (x - slope) >> 16;
  }
  
  edge->x_range = get_range(a.x, b.x);
}

#define EDGE(_edge) raster_edge[edge_index[_edge]]

#define REGION_OFFSET(_region, _y) (_y - _region->min_y)

_Bool edge_start_x(X3D_RasterEdge* edge, int16* x) {
  if(edge->flags & EDGE_INVISIBLE) {
    return FALSE;
  }
  if(edge->flags & EDGE_HORIZONTAL) {
    return FALSE;
  }
  
  *x = edge->x_data[0];
  return TRUE;
}

void draw_edge(X3D_RasterEdge* edge) {
  if(edge->flags & EDGE_INVISIBLE)
    return;
  
  
  if(edge->flags & EDGE_HORIZONTAL) {
    FastDrawHLine(LCD_MEM, edge->x_range.min, edge->x_range.max, edge->y_range.min, A_XOR);
  }
  else {
    int16 y = edge->y_range.min;
    
    while(y <= edge->y_range.max) {
      DrawPix(edge->x_data[y - edge->y_range.min], y, A_XOR);
      ++y;
    }
  }
}

_Bool get_rasterregion(X3D_RasterRegion* region, X3D_RenderStack* stack, X3D_RasterEdge raster_edge[], int16 edge_index[], int16 total_e) {
  region->y_range.min = INT16_MAX;
  region->y_range.max = INT16_MIN;
  
  region->x_left = renderstack_alloc(stack, LCD_HEIGHT * sizeof(int16));
  region->x_right = renderstack_alloc(stack, LCD_HEIGHT * sizeof(int16));
  
  int16 i;
  for(i = 0; i < LCD_HEIGHT; ++i) {
    region->x_left[i] = 1000;//INT16_MAX;
    region->x_right[i] = -1000;//INT16_MIN;
  }
  
  //printf("total_e: %d\n", total_e);
  
  Vex2D out_v[2];
  int16 total_out_v = 0;
  X3D_RasterEdge* e = NULL;
  
  X3D_RasterEdge temp_edge;
  
  int16 edge;
  for(edge = 0; edge < total_e; ++edge) {
    e = &EDGE(edge);
    
    if(e->flags & EDGE_NEAR_CLIPPED) {
      //ASSERT((e->flags & EDGE_HORIZONTAL) == 0);
      //ASSERT(e->x_data);
      
      if(total_out_v == 2) {
        x3d_error("%d %d %d %d", EDGE(0).flags, EDGE(1).flags, EDGE(2).flags, EDGE(3).flags);
      }
      
      int16 a_x, b_x;
      
      if(e->flags & EDGE_INVISIBLE) {
        a_x = e->start_x;
        b_x = e->end_x;
      }
      else {
        a_x = e->x_data[0];
        b_x = EDGE_VALUE(e, e->y_range.max);
      }
      
      //ASSERT(total_out_v < 2);
      
      
      if(e->flags & EDGE_HORIZONTAL) {
        out_v[total_out_v++] = (Vex2D) { (e->flags & EDGE_V_SWAPPED) ? e->x_range.min : e->x_range.max, e->y_range.min };
      }
      else {
        if(e->flags & EDGE_V_SWAPPED) {
          out_v[total_out_v++] = (Vex2D) { a_x, e->y_range.min };
        }
        else {
          out_v[total_out_v++] = (Vex2D) { b_x , e->y_range.max };
        }
      }
    }
    
add_edge:
    if(!(e->flags & EDGE_INVISIBLE)) {
      int16* left = region->x_left + e->y_range.min;
      int16* right = region->x_right + e->y_range.min;
      
      if(e->flags & EDGE_HORIZONTAL) {
        if(e->x_range.min < *left)    *left = e->x_range.min;
        if(e->x_range.max > *right)   *right = e->x_range.max;
      }
      else {
        int16 i;
        int16* x = e->x_data;
        
        //draw_edge(e);
      
        for(i = e->y_range.min; i <= e->y_range.max; ++i) {
          if(*x < *left)    *left = *x;
          if(*x > *right)   *right = *x;
          
          ++x;
          ++left;
          ++right;
        }
      }
      
      region->y_range.min = min(region->y_range.min, e->y_range.min);
      region->y_range.max = max(region->y_range.max, e->y_range.max);
    }
    else {
      //printf("Invisible!\n");
    }
  }
  
  ASSERT(total_out_v == 0 || total_out_v == 2);
  
  if(total_out_v != 0 && e != &temp_edge) {
    
    
    
//     x3d_error("Range: {%d, %d}\n{%d, %d}\n{%d, %d}\n{%d, %d}\n",
//               EDGE(0).y_range.min,EDGE(0).y_range.max,
//               EDGE(1).y_range.min,EDGE(1).y_range.max,
//               EDGE(2).y_range.min,EDGE(2).y_range.max,
//               EDGE(3).y_range.min,EDGE(3).y_range.max);
    
    generate_rasteredge(stack, &temp_edge, out_v[0], out_v[1], region->y_range);
    
    //x3d_error("Pos: {%d, %d} - {%d, %d}", out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y);
    
    
    e = &temp_edge;
    goto add_edge;
  }
  
  //printf("Min: %d, %d\n", region->y_range.min, region->y_range.max);
  
  if(region->y_range.min <= region->y_range.max) {
    region->x_left += region->y_range.min;
    region->x_right += region->y_range.min;
    return TRUE;
  }
  
  return FALSE;
}

void rasterize_rasterregion(X3D_RasterRegion* region, void* screen, uint16 color) {
  int16 y = region->y_range.min;
  
  //return;

  uint8* span = screen + y * (LCD_WIDTH / 8);
  
  void (*render_span)(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0")) = (void* []) {
    GrayDrawSpan_WHITE_R,
    GrayDrawSpan_LGRAY_R,
    GrayDrawSpan_DGRAY_R,
    GrayDrawSpan_BLACK_R
  }[color];
  
  printf("y: %d, %d\n", region->y_range.min, region->y_range.max);
  
  while(y <= region->y_range.max) {
    
    
    render_span(region->x_left[y - region->y_range.min], region->x_right[y - region->y_range.min], span);
    
    span += LCD_WIDTH / 8;
    
    //FastDrawHLine(LCD_MEM, region->x_left[y - region->min_y], region->x_right[y - region->min_y], y, A_XOR);
    ++y;
  }
}










void test_newnew_clip() {
  X3D_RenderStack stack;
  renderstack_init(&stack, 2048);
  FontSetSys(F_4x6);
  
  // Case 0
  {
    void* stack_ptr = renderstack_save(&stack);
    X3D_Range y_range = SCREEN_Y_RANGE;
     X3D_RasterEdge edge;
    
    Vex2D a = { -20000, -30000 };
    Vex2D b = { 20000, -30000 };
    
    generate_rasteredge(&stack, &edge, a, b, y_range);
    
    printf("Case 0\n");
    ASSERT_EQUAL_INT16((edge.flags & EDGE_INVISIBLE) != 0, TRUE);
    ASSERT_EQUAL_INT16((edge.flags & EDGE_HORIZONTAL) != 0, TRUE);
    ASSERT_EQUAL_INT16(edge.x_range.min, -20000);
    ASSERT_EQUAL_INT16(edge.x_range.max, 20000);
    ASSERT_EQUAL_INT16(edge.y_range.min, -30000);
    ASSERT_EQUAL_INT16(edge.y_range.max, -30000);
    ASSERT_EQUAL_INT32(edge.x_data, NULL);
    
    renderstack_restore(&stack, stack_ptr);
  }
  
  // Case 1
  {
    void* stack_ptr = renderstack_save(&stack);
    X3D_Range y_range = SCREEN_Y_RANGE;
    X3D_RasterEdge edge;
    
    Vex2D a = { -20000, 30000 };
    Vex2D b = { 20000, 30000 };
    
    generate_rasteredge(&stack, &edge, a, b, y_range);
    
    printf("Case 1\n");
    ASSERT_EQUAL_INT16((edge.flags & EDGE_INVISIBLE) != 0, TRUE);
    ASSERT_EQUAL_INT16((edge.flags & EDGE_HORIZONTAL) != 0, TRUE);
    ASSERT_EQUAL_INT16(edge.x_range.min, -20000);
    ASSERT_EQUAL_INT16(edge.x_range.max, 20000);
    ASSERT_EQUAL_INT16(edge.y_range.min, 30000);
    ASSERT_EQUAL_INT16(edge.y_range.max, 30000);
    ASSERT_EQUAL_INT32(edge.x_data, NULL);
    
    renderstack_restore(&stack, stack_ptr);
  }
  
  // Case 2
  {
    void* stack_ptr = renderstack_save(&stack);
    X3D_Range y_range = SCREEN_Y_RANGE;
    X3D_RasterEdge edge;
    
    Vex2D a = { 50, 51 };
    Vex2D b = { 100, 101 };
    
    generate_rasteredge(&stack, &edge, a, b, y_range);
    
    printf("Case 2\n");
    ASSERT_EQUAL_INT16(edge.flags & EDGE_INVISIBLE, 0);
    ASSERT_EQUAL_INT16(edge.flags & EDGE_HORIZONTAL, 0);
    
    
    ASSERT_EQUAL_INT16(edge.x_range.min, 50);
    ASSERT_EQUAL_INT16(edge.x_range.max, 100);
    ASSERT_EQUAL_INT16(edge.y_range.min, 51);
    ASSERT_EQUAL_INT16(edge.y_range.max, 101);
    ASSERT_EQUAL_INT32(edge.x_data != NULL, TRUE);
    
    ASSERT_EQUAL_INT16(edge.x_data[0], 50);
    ASSERT_EQUAL_INT16(edge.x_data[50], 100);
    ASSERT_EQUAL_INT16(edge.x_data[25], (edge.x_data[0] + edge.x_data[50]) / 2);
    
    
    renderstack_restore(&stack, stack_ptr);
  }
  
  // Case 3
  {
    void* stack_ptr = renderstack_save(&stack);
    X3D_Range y_range = SCREEN_Y_RANGE;
    X3D_RasterEdge edge;
    
    Vex2D a = { 0, -10 };
    Vex2D b = { 20, 10 };
    
    generate_rasteredge(&stack, &edge, a, b, y_range);
    
    printf("Case 3\n");
    ASSERT_EQUAL_INT16(edge.flags & EDGE_INVISIBLE, 0);
    ASSERT_EQUAL_INT16(edge.flags & EDGE_HORIZONTAL, 0);
    
    
    ASSERT_EQUAL_INT16(edge.x_range.min, 10);
    ASSERT_EQUAL_INT16(edge.x_range.max, 20);
    ASSERT_EQUAL_INT16(edge.y_range.min, 0);
    ASSERT_EQUAL_INT16(edge.y_range.max, 10);
    ASSERT_EQUAL_INT32(edge.x_data != NULL, TRUE);
    
    ASSERT_EQUAL_INT16(edge.x_data[0], 10);
    ASSERT_EQUAL_INT16(edge.x_data[10], 20);
    ASSERT_EQUAL_INT16(edge.x_data[5], (edge.x_data[0] + edge.x_data[10]) / 2);
    
    
    renderstack_restore(&stack, stack_ptr);
  }
}

//_Bool get_rasterregion(X3D_RasterRegion* region, X3D_RenderStack* stack, X3D_RasterEdge raster_edge[], int16 edge_index[], int16 total_e)

_Bool construct_rasterregion_from_points(X3D_RenderStack* stack, X3D_RasterRegion* dest, Vex2D* v, uint16 total_v) {
  X3D_RasterEdge edges[total_v];
  uint16 edge_index[total_v];
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    int16 next = (i + 1) % total_v;
    generate_rasteredge(stack, edges + i, v[i], v[next], (X3D_Range) { 0, LCD_HEIGHT - 1 });
    
    //printf("es: %d, ee: %d\n", edges[i].min_y, edges[i].max_y);
    
    edge_index[i] = i;
  }
  
  //printf("Min: %d, %d\n", dest->y_range.min, dest-> 
  
  return get_rasterregion(dest, stack, edges, edge_index, total_v);
}

void x3d_init_clip_window(X3D_RenderStack* stack, X3D_Context* context, X3D_RasterRegion* region, Vex2D* v, uint16 total_v) {
  construct_rasterregion_from_points(stack, region, v, total_v);
}

#define CLIP() clip_span(*portal_left, *portal_right, region_left, region_right)

// Clips a single span against another span (from a portal polygon)
_Bool clip_span(int16 portal_left, int16 portal_right, int16* span_left, int16* span_right) {
  *span_left = max(portal_left, *span_left);
  *span_right = min(portal_right, *span_right);
  
  return *span_left <= *span_right;
}

_Bool intersect_rasterregion(X3D_RasterRegion* portal, X3D_RasterRegion* region) {
  int16* portal_left = portal->x_left + region->y_range.min - portal->y_range.min;
  int16* portal_right = portal->x_right + region->y_range.min - portal->y_range.min;
  
  int16* region_left = region->x_left;
  int16* region_right = region->x_right;
  
  ASSERT(region_left);
  ASSERT(region_right);
  ASSERT(portal_left);
  ASSERT(region_right);
  
  int16 y = region->y_range.min;

  //return FALSE;
  
  
  //ASSERT(y >= 0 && y < LCD_HEIGHT);
  
  if(y < 0 || y >= LCD_HEIGHT) {
    x3d_error("y = %d, %d", y, region->y_range.max);
  }
  
  ASSERT_RANGE(y, region->y_range.min, region->y_range.max);
  
  // Skip fully clipped spans
  while(y <= region->y_range.max && !CLIP()) {
    ++y;
    ++portal_left;
    ++portal_right;
    ++region_left;
    ++region_right;
  }
  
  // If all the spans are fully clipped, it's invisible!
  if(y > region->y_range.max) {
    return FALSE;
  }
  
  // We need to actually adjust x_left and x_right to point to the first visible span
  region->x_left = region_left;
  region->x_right = region_right;
  
  region->y_range.min = y;
  
  ENTER();
  while(y <= region->y_range.max && CLIP()) {
    ++y;
    ++portal_left;
    ++portal_right;
    ++region_left;
    ++region_right;
  }
  EXIT();
  
  region->y_range.max = y - 1;
  
  return TRUE;
}





















