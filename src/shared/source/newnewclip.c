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

#include "extgraph.h"

#define ASSERT(_s) {if(!(_s)) x3d_error("Assert failed! %s", #_s);}
#define ASSERT_RANGE(_a, _min, _max) ASSERT(_a >= _min && _a <= _max);

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

void draw_edge(X3D_RasterEdge* edge) {
  if(edge->flags & EDGE_INVISIBLE)
    return;
  
  
  if(edge->flags & EDGE_HORIZONTAL) {
    FastDrawHLine(LCD_MEM, edge->min_x, edge->max_x, edge->min_y, A_XOR);
  }
  else {
    int16 y = edge->min_y;
    
    while(y <= edge->max_y) {
      DrawPix(edge->x_data[y - edge->min_y], y, A_XOR);
      ++y;
    }
  }
}

void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, int16 min_y, int16 max_y) {
  // Swap points if out of order
  if(a.y > b.y) {
    SWAP(a, b);
  }
  
  edge->flags = 0;
  
  if(b.y < min_y || a.y > max_y) {
    edge->flags = EDGE_INVISIBLE;
    edge->min_y = a.y;
    edge->max_y = b.y;
  }
  
  if(a.y == b.y) {
    // Horizontal line
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
    edge->max_y = a.y;
  }
  else if(!(edge->flags & EDGE_INVISIBLE)) {
    // Regular line
    fp16x16 x;
    int16 y;
    fp16x16 slope = vertical_slope(a, b);
    
    // Check if the line needs to be clipped against the line y = y_min
    if(a.y >= min_y) {
      x = ((fp16x16)a.x) << 16;
      y = a.y;
    }
    else {
      // Clip the line
      fp8x8 new_slope = slope >> 8;
      
      x = (((int32)a.x << 8) + ((int32)(min_y - a.y) * new_slope)) << 8;
      y = min_y;
    }
    
    // Clamp the max y
    int16 end_y = min(max_y, b.y);
    int16 start_x = x >> 16;
    
    edge->min_y = y;
    edge->flags = 0;
    
    // Allocate space for the values
    edge->x_data = renderstack_alloc(stack, (end_y - y + 1) * 2);

    
    ENTER();

    while(y <= end_y) {
      edge->x_data[y - edge->min_y] = x >> 16;
      x += slope;
      ++y;
    }
    EXIT();
    
    int16 end_x = (x - slope) >> 16;
    
    edge->min_x = min(start_x, end_x);
    edge->max_x = max(start_x, end_x);
    
    
    edge->max_y = end_y;
    
  }
}


uint16 find_top_edge(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e) {
  int16 min_index = 0;
  uint16 i;
  
  for(i = 1; i < total_e; ++i) {
    uint16 index = edge_index[i];
    X3D_RasterEdge* edge = raster_edge + index;
    X3D_RasterEdge* min_edge = raster_edge + edge_index[min_index];
    
    if(edge->min_y < min_edge->min_y) {
      min_index = i;
    }
    else if(edge->min_y == min_edge->min_y) {
      // Only update the edge if it's not horizontal and it's more left than the current one
      if((edge->flags & EDGE_HORIZONTAL) || (!(min_edge->flags & EDGE_HORIZONTAL) && edge->x_data[1] < min_edge->x_data[1])) {
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
      if((edge->flags & EDGE_HORIZONTAL) || (!(max_edge->flags & EDGE_HORIZONTAL) && edge->x_data[1] < max_edge->x_data[1])) {
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

int16* populate_edge(X3D_RasterEdge* edge, int16* dest, _Bool first_edge, _Bool left) {
  if(edge->flags & EDGE_HORIZONTAL) {
    if(first_edge) {
      *dest = left ? edge->min_x : edge->max_x;
      
      return dest + 1;
    }
    else {
      return dest;
    }
  }
  else {
    int16 count = edge->max_y - edge->min_y;
    int16* data = edge->x_data;
    
    if(first_edge) {
      ++count;
    }
    else {
      ++data;
    }
    
    printf("Count: %d\n", count);
    
    ASSERT_RANGE(count, 0, 128);
    
    int16 total_count = count;

    while(count-- > 0) {
      *dest = *data;
      
      ++dest;
      ++data;
    }
    EXIT();
    
    return dest;
  }
}

#define EDGE_INDEX() (edge_index[*edge])
#define NEXT_EDGE() (edge_index[edge[1]])

#define EDGE() raster_edge[EDGE_INDEX()]

int16 populate_polyline(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, uint16 start_edge,
                        int16 dir, uint16 end_edge, int16* dest, _Bool left, int16* start_y) {
  
  int16* start = dest;
  uint16 edge_list[20];
  uint16 total_list = 0;
  
  int16 end;
  
  uint16 index = start_edge;
  
  if(!left && start_edge == end_edge) {
    // If we're the side and we only contain one edge, just add it
    edge_list[total_list++] = index;
    index = next_edge(index, total_e, dir);
  }
  else {
    // Otherwise, add all the edges up until the last edge
    ENTER();
    while(index != end_edge) {
      edge_list[total_list++] = index;
      index = next_edge(index, total_e, dir);
    }
    EXIT();
  }
  
  // Only add the bottom edge if we're the left side
  // This is because the bottom edge is guaranteed to be on the left
  if(left) {
    edge_list[total_list++] = index;
    index = next_edge(index, total_e, dir);
  }
  
  uint16* edge = edge_list;
  uint16* edge_end = edge_list + total_list;

  // Skip invisible edges
  ENTER();
  while(edge < edge_end && (EDGE().flags & EDGE_INVISIBLE)) {
    printf("Skipped!\n");
    ++edge;
  }
  EXIT();
  
  // All edges are invisible
  if(edge == edge_end) {
    x3d_error("Invisible!");
    *start_y = 10000;
    return 0;
  }
  
  // Calculate the starting y value
  *start_y = EDGE().min_y;
    
  _Bool last = FALSE;
  _Bool first = TRUE;
  
  ENTER();
  do {
    // We're the last edge if we're out of edges or the next edge is invisible
    last = edge + 1 == edge_end || (EDGE().flags & EDGE_INVISIBLE);
    dest = populate_edge(&EDGE(), dest, first, left);
    ++edge;
    first = FALSE;
  } while(edge < edge_end && !last);
  EXIT();
  
  // Return how many values were written in this polyline
  return dest - start;
}

// Clips a single span against another span (from a portal polygon)
_Bool clip_span(int16 portal_left, int16 portal_right, int16* span_left, int16* span_right) {
  *span_left = max(portal_left, *span_left);
  *span_right = min(portal_right, *span_right);
  
  return *span_left <= *span_right;
}

#define CLIP() clip_span(*portal_left, *portal_right, region_left, region_right)

void rasterize_rasterregion(X3D_RasterRegion* region, void* screen, uint16 color) {
  int16 y = region->min_y;

  uint8* span = screen + y * (LCD_WIDTH / 8);
  
  void (*render_span)(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0")) = (void* []) {
    GrayDrawSpan_WHITE_R,
    GrayDrawSpan_LGRAY_R,
    GrayDrawSpan_DGRAY_R,
    GrayDrawSpan_BLACK_R
  }[color];
  
  printf("y: %d, %d\n", region->min_y, region->max_y);
  
  ASSERT_RANGE(y, 0, 127);
  
  ENTER();
  while(y <= region->max_y) {
    
    
    render_span(region->x_left[y - region->min_y], region->x_right[y - region->min_y], span);
    
    span += LCD_WIDTH / 8;
    
    //FastDrawHLine(LCD_MEM, region->x_left[y - region->min_y], region->x_right[y - region->min_y], y, A_XOR);
    ++y;
  }
  EXIT();
}


_Bool intersect_rasterregion(X3D_RasterRegion* portal, X3D_RasterRegion* region) {
  int16* portal_left = portal->x_left + region->min_y - portal->min_y;
  int16* portal_right = portal->x_right + region->min_y - portal->min_y;
  
  int16* region_left = region->x_left;
  int16* region_right = region->x_right;
  
  ASSERT(region_left);
  ASSERT(region_right);
  ASSERT(portal_left);
  ASSERT(region_right);
  
  int16 y = region->min_y;  
  
  ASSERT(y >= 0 && y < LCD_HEIGHT);
  
  ASSERT_RANGE(y, region->min_y, region->max_y);
  
  // Skip fully clipped spans
  ENTER();
  while(y <= region->max_y && !CLIP()) {
    ++y;
    ++portal_left;
    ++portal_right;
    ++region_left;
    ++region_right;
  }
  EXIT();
  
  // If all the spans are fully clipped, it's invisible!
  if(y > region->max_y) {
    return FALSE;
  }
  
  
  // We need to actually adjust x_left and x_right to point to the first visible span
  region->x_left = region_left;
  region->x_right = region_right;
  
  region->min_y = y;
  
  ENTER();
  while(y <= region->max_y && CLIP()) {
    ++y;
    ++portal_left;
    ++portal_right;
    ++region_left;
    ++region_right;
  }
  EXIT();
  
  region->max_y = y - 1;
  
  return TRUE;
}

_Bool construct_rasterregion(X3D_RenderStack* stack, X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, X3D_RasterRegion* dest, int16 min_y, int16 max_y) {
  
  // Find the top
  uint16 top_index = find_top_edge(raster_edge, edge_index, total_e);
  X3D_RasterEdge* top = raster_edge + edge_index[top_index];
  
  
  ADDRESS(top);
    
  // Find the bottom
  uint16 bottom_index = find_bottom_edge(raster_edge, edge_index, total_e);
  X3D_RasterEdge* bottom = raster_edge + edge_index[bottom_index];
  
  printf("top: %d, %d\n", top_index, bottom_index);
  
  ADDRESS(bottom);
  
  // Get the next edge from the top in either direction
  X3D_RasterEdge* next_left = raster_edge + edge_index[next_edge(top_index, total_e, -1)];
  X3D_RasterEdge* next_right = raster_edge + edge_index[next_edge(top_index, total_e, 1)];
  
  ADDRESS(next_left);
  ADDRESS(next_right);
  
  // Calculate the maximum estimated height of the region
  // Note: it can never be more than this
  int16 estimated_height = min(max_y, bottom->max_y) - max(min_y, top->min_y) + 1;
  
  if(estimated_height <= 0) {
    //x3d_error("NEG HEIGHT: %d, %d, %d, %d", max_y, bottom->max_y, min_y, top->min_y);
    return FALSE;
  }
  
  
  // Allocate space for each polyline (on the left and right)
  int16* left = renderstack_alloc(stack, estimated_height * 2);
  int16* right = renderstack_alloc(stack, estimated_height * 2);
  
  ADDRESS(left);
  ADDRESS(right);
  
  if(estimated_height == 1) {
    printf("Height = 1\n");
    // Height of 1 pixel (top and bottom must be horizontal)
    // Yuck, all y values are the same! We have to search for
    // min x and max y
    
    uint16 i;
    
    int16 min_x = 10000;
    int16 max_x = -10000;
    
    for(i = 0; i < total_e; ++i) {
      //ASSERT(raster_edge[edge_index[i]].flags & EDGE_HORIZONTAL);
      
      min_x = min(min_x, raster_edge[edge_index[i]].min_x);
      max_x = max(max_x, raster_edge[edge_index[i]].max_x);
    }
    
    printf("Minx: %d, %d\n", min_x, max_x);
    
    *left = min_x;
    *right = max_x;
    
    dest->x_left = left;
    dest->x_right = right;
    dest->min_y = top->min_y;
    dest->max_y = top->min_y;
    
    return TRUE;
  }
  else if(estimated_height == 2) {
    printf("Height = 2\n");
    // Height of 2 pixels (top and bottom must be horizontal)
    left[0] = top->min_x;
    right[0] = top->max_x;
    
    left[1] = bottom->min_x;
    right[1] = bottom->max_x;
    
    dest->x_left = left;
    dest->x_right = right;
    
    dest->min_y = top->min_y;
    dest->max_y = top->min_y + 1;
    
    return TRUE;
  }
  
  //return FALSE;
  
  // Starting edge for the right side
  int16 start_right;

  // The direction we need to walk through the edges for the left polyline
  // This is because a polygon could be either clockwise or counter-clockwise
  int16 left_dir;
  
  if(top->flags & EDGE_HORIZONTAL) {
    // If the top is horizontal, both the left and right polyline need to start on
    // the horizontal edge
    start_right = top_index;
    
    // Determine whether the left direction is really next_left or next_right.
    // If next_left is actually on the left, left_dir should be -1. Otherwise,
    // it's flipped and should be 1.
    
    
    int16 x_left, x_right;
    
    if(next_left->flags & EDGE_HORIZONTAL) {
      x_left = next_left->min_x;
    }
    else {
      x_left = next_left->x_data[1];
    }
    
    if(next_right->flags & EDGE_HORIZONTAL) {
      x_right = next_right->min_x;
    }
    else {
      x_right = next_right->x_data[1];
    }
    
    left_dir = (x_left < x_right ? -1 : 1);
  }
  else {
    // Since this isn't a flat-top polygon, the top edge is guaranteed to point to
    // the top edge for the right. If the next_right edge also has the same min_y
    // as the top edge, this means next_left and next_right are correct and left_dir
    // should be -1. Otherwise, we're proceeding in the wrong direction and left_dir
    // should be 1.
    if(next_right->min_y == top->min_y) {
      left_dir = -1;
    }
    else {
      left_dir = 1;
    }

    // The starting right edge should be the first edge in the reverse
    // left_dir direction.
    start_right = next_edge(top_index, total_e, -left_dir);
    
  }
  
  // Populate the left and right polylines
  int16 total_left = populate_polyline(raster_edge, edge_index, total_e, top_index, left_dir, bottom_index, left, TRUE, &dest->min_y);
  int16 total_right = populate_polyline(raster_edge, edge_index, total_e, start_right, -left_dir, bottom_index, right, FALSE, &dest->min_y);
  
  //int16 size_top_right = abs(right[1] - right[2]);
  
 // if(abs(right[]))
  
  
  // If the height of the left is not the same height as the right, we have a problem!
  if(total_left != total_right) {
    //x3d_error("Polgon polyline mismatch (left: %d, right: %d)", total_left, total_right);
    
    printf("Mismatch: (e = %d)\n", total_e);
  }
  
  dest->x_left = left;
  dest->x_right = right;
  
  // Set the y range for the raster region
  dest->min_y = dest->min_y;
  dest->max_y = dest->min_y + total_left - 1;

  // If max_y < min_y we have an invalid raster region!
  return dest->max_y >= dest->min_y;
}


_Bool construct_and_clip_rasterregion(X3D_RenderStack* stack, X3D_RasterRegion* portal, X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, X3D_RasterRegion* dest) {  
  return construct_rasterregion(stack, raster_edge, edge_index, total_e, dest, portal->min_y, portal->max_y)
  
    && intersect_rasterregion(portal, dest);
}

_Bool construct_rasterregion_from_points(X3D_RenderStack* stack, X3D_RasterRegion* dest, Vex2D* v, uint16 total_v) {
  X3D_RasterEdge edges[total_v];
  uint16 edge_index[total_v];
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    int16 next = (i + 1) % total_v;
    generate_rasteredge(stack, edges + i, v[i], v[next], 0, LCD_HEIGHT - 1);
    
    //printf("es: %d, ee: %d\n", edges[i].min_y, edges[i].max_y);
    
    edge_index[i] = i;
  }
  
  return construct_rasterregion(stack, edges, edge_index, total_v, dest, 0, LCD_HEIGHT - 1);
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
  
  X3D_RasterRegion screen_region;

  // Construct a test clipping portal
  Vex2D screen_v[] = {
    { 120, 120 },
    { 10, 122 },
    { LCD_WIDTH - 10, 122 }
  };
    
  construct_rasterregion_from_points(&stack, &screen_region, screen_v, 3);  
  rasterize_rasterregion(&screen_region, LCD_MEM, 2);
  
  ngetchx();
   
  
  // Construct a polygon to clip against the portal
  Vex2D v[] = {
    { 200, 110 },
    { 228, 70 },
    { 90, 30 },
    { 20, 30 },
    { 120, 110 }
  };
  
  
  uint16 i;

  // Number of points in the polygon
  uint16 TOTAL = 5;
  
  X3D_RasterEdge edges[20];
  
  for(i = 0; i < TOTAL; ++i) {
    int16 next = (i + 1) % TOTAL;
    generate_rasteredge(&stack, edges + i + 5, v[i], v[next], screen_region.min_y, screen_region.max_y);
    
    draw_edge(edges + i + 5);
    //ngetchx();
  }
  
  X3D_RasterRegion region;
  
  uint16 edge_index[] =  { 5, 6, 7, 8, 9 };
  
  clrscr();
  
  if(construct_and_clip_rasterregion(&stack, &screen_region, edges, edge_index, 5, &region)) {
    rasterize_rasterregion(&region, LCD_MEM, 0);
  }
  
  ngetchx();
  
  return;  
}


void x3d_init_clip_window(X3D_RenderStack* stack, X3D_Context* context, X3D_RasterRegion* region, Vex2D* v, uint16 total_v) {
  construct_rasterregion_from_points(stack, region, v, total_v);
}



































