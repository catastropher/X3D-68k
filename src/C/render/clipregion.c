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

#include "X3D_common.h"
#include "X3D_segment.h"
#include "X3D_vector.h"
#include "X3D_render.h"
#include "X3D_trig.h"
#include "X3D_clipregion.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"
#include "memory/X3D_stack.h"
#include "X3D_assert.h"

#if 0

fp16x16 vertical_slope(X3D_Vex2D v1, X3D_Vex2D v2) {
  if(v1.y == v2.y)
    return 0;
  
  return (((int32)(v2.x - v1.x)) << 16) / (v2.y - v1.y);
}

void intersect_line_with_horizontal(fp16x16 slope, X3D_Vex2D* start, int16 y) {
  ASSERT((slope >> 16) < 128);    // To prevent overflow when converting to fp8x8 for the slope
  
  int16 dy = y - start->y;
  int16 slope_8x8 = slope >> 8;
  
  start->x = start->x + (((int32)dy * slope_8x8) >> 8);
  start->y = y;
}

_Bool clip_rasteredge(X3D_RasterEdge* edge, X3D_Vex2D* a, X3D_Vex2D* b, fp16x16* slope, X3D_Range region_y_range) {
  edge->flags = 0;
  edge->x_data = NULL;
  
  edge->start_x = a->x;
  edge->end_x = b->x;
  
  // Swap points if out of order vertically
  if(a->y > b->y) {
    X3D_SWAP(*a, *b);
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
    
    return X3D_FALSE;
  }
  
  if(!(edge->flags & EDGE_HORIZONTAL)) {         // Only clip visible and non-horizontal edges
    *slope = vertical_slope(*a, *b);
    
    if(a->y < region_y_range.min) {
      intersect_line_with_horizontal(*slope, a, region_y_range.min);
      edge->y_range.min = region_y_range.min;
    }
    
    // Clamp the max y
    edge->y_range.max = b->y = X3D_MIN(edge->y_range.max, region_y_range.max);
  }
  
  return X3D_TRUE;
}

#define EDGE_VALUE(_edge, _y) ((_edge)->x_data[_y - (_edge)->y_range.min])
 
void x3d_rasteredge_generate(X3D_Stack* stack, X3D_RasterEdge* edge, X3D_Vex2D a, X3D_Vex2D b, X3D_Range region_y_range) {
  fp16x16 slope;
  
  edge->start = a;
  edge->end = b;
  
  //ASSERT(region_y_range.min >= 0 && region_y_range.max < LCD_HEIGHT);
  
  if(!(region_y_range.min >= 0 && region_y_range.max < LCD_HEIGHT)) {
    x3d_error("range min: %d, %d", region_y_range.min, region_y_range.max);
  }
  
  //printf("a: %d, b: %d\n", a.x, b.x);
  
  if(clip_rasteredge(edge, &a, &b, &slope, region_y_range)) {     // Only generate the edge if it's (potentially) visible
    fp16x16 x = ((int32)a.x) * 65536L;
    int16 y = a.y;
    int16 height = b.y - a.y + 1;
    
    ASSERT(region_y_range.min <= region_y_range.max);
    ASSERT(in_range(SCREEN_Y_RANGE, height - 1));
    ASSERT(in_range(region_y_range, a.y));
    ASSERT(in_range(region_y_range, b.y));
    
    // Allocate space for the values
    edge->x_data = x3d_stack_alloc(stack, height * 2);
    
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
    return X3D_FALSE;
  }
  if(edge->flags & EDGE_HORIZONTAL) {
    return X3D_FALSE;
  }
  
  *x = edge->x_data[0];
  return X3D_TRUE;
}

void draw_edge(X3D_RasterEdge* edge) {
#if 0
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
#endif
}

_Bool x3d_clipregion_construct_from_edges(X3D_RasterRegion* region, X3D_Stack* stack, X3D_RasterEdge raster_edge[], int16 edge_index[], int16 total_e) {
  region->y_range.min = INT16_MAX;
  region->y_range.max = INT16_MIN;
  
  region->x_left = x3d_stack_alloc(stack, LCD_HEIGHT * sizeof(int16));
  region->x_right = x3d_stack_alloc(stack, LCD_HEIGHT * sizeof(int16));
  
  int16 i;
  for(i = 0; i < LCD_HEIGHT; ++i) {
    region->x_left[i] = 1000;//INT16_MAX;
    region->x_right[i] = -1000;//INT16_MIN;
  }
  
  //printf("total_e: %d\n", total_e);
  
  X3D_Vex2D out_v[20];
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
        out_v[total_out_v++] = (X3D_Vex2D) { (e->flags & EDGE_V_SWAPPED) ? e->x_range.min : e->x_range.max, e->y_range.min };
      }
      else {
        if(e->flags & EDGE_V_SWAPPED) {
          out_v[total_out_v++] = (X3D_Vex2D) { a_x, e->y_range.min };
        }
        else {
          out_v[total_out_v++] = (X3D_Vex2D) { b_x , e->y_range.max };
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
      
      region->y_range.min = X3D_MIN(region->y_range.min, e->y_range.min);
      region->y_range.max = X3D_MAX(region->y_range.max, e->y_range.max);
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
    
    x3d_rasteredge_generate(stack, &temp_edge, out_v[0], out_v[1], region->y_range);
    
    //x3d_error("Pos: {%d, %d} - {%d, %d}", out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y);
    
    //x3d_log(X3D_WARN, "Doing stupid things\n");
    
    e = &temp_edge;
    //goto add_edge;
  }
  
  //printf("Min: %d, %d\n", region->y_range.min, region->y_range.max);
  
  if(region->y_range.min <= region->y_range.max) {
    region->x_left += region->y_range.min;
    region->x_right += region->y_range.min;
    return X3D_TRUE;
  }
  
  return X3D_FALSE;
}

void rasterize_clipregion(X3D_RasterRegion* region, void* screen, uint16 color) {
#if 0
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
#endif
}


//_Bool get_clipregion(X3D_RasterRegion* region, X3D_Stack* stack, X3D_RasterEdge raster_edge[], int16 edge_index[], int16 total_e)

///////////////////////////////////////////////////////////////////////////////
/// Constructs a raster region from a list of points that define a 2D polygon.
///
/// @param stack    - render stack to construct the raster region on
/// @param dest     - raster region
/// @param v        - list of 2D points
/// @param total_v  - number of points
///
/// @return Whether a valid region was created.
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_clipregion_construct_from_points(X3D_Stack* stack, X3D_RasterRegion* dest, X3D_Vex2D* v, uint16 total_v) {
  X3D_RasterEdge edges[total_v];
  uint16 edge_index[total_v];
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    int16 next = (i + 1) % total_v;
    x3d_rasteredge_generate(stack, edges + i, v[i], v[next], (X3D_Range) { 0, LCD_HEIGHT - 1 });
    
    //printf("es: %d, ee: %d\n", edges[i].min_y, edges[i].max_y);
    
    edge_index[i] = i;
  }
  
  //printf("Min: %d, %d\n", dest->y_range.min, dest-> 
  
  return x3d_clipregion_construct_from_edges(dest, stack, edges, edge_index, total_v);
}

#if 0
void x3d_init_clip_window(X3D_Stack* stack, X3D_Context* context, X3D_RasterRegion* region, X3D_Vex2D* v, uint16 total_v) {
  construct_clipregion_from_points(stack, region, v, total_v);
}
#endif

#define CLIP() clip_span(*portal_left, *portal_right, region_left, region_right)

// Clips a single span against another span (from a portal polygon)
_Bool clip_span(int16 portal_left, int16 portal_right, int16* span_left, int16* span_right) {
  *span_left = X3D_MAX(portal_left, *span_left);
  *span_right = X3D_MIN(portal_right, *span_right);
  
  return *span_left <= *span_right;
}

_Bool x3d_clipregion_intersect(X3D_RasterRegion* portal, X3D_RasterRegion* region) {
  int16* portal_left = portal->x_left + region->y_range.min - portal->y_range.min;
  int16* portal_right = portal->x_right + region->y_range.min - portal->y_range.min;
  
  int16* region_left = region->x_left;
  int16* region_right = region->x_right;
  
  ASSERT(region_left);
  ASSERT(region_right);
  ASSERT(portal_left);
  ASSERT(region_right);
  
  int16 y = region->y_range.min;

  //return X3D_FALSE;
  
  
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
    return X3D_FALSE;
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
  
  return X3D_TRUE;
}

static _Bool clipregion_point_inside(X3D_RasterRegion* region, X3D_Vex2D p) {
  if(p.y < region->y_range.min || p.y > region->y_range.max)
    return X3D_FALSE;
  
  uint16 offsety = p.y - region->y_range.min;
  
  return p.x >= region->x_left[offsety] && p.x <= region->x_right[offsety];
}


int16 x3d_clip_line_to_near_plane(X3D_Vex3D* a, X3D_Vex3D* b, X3D_Vex2D* a_project, X3D_Vex2D* b_project, X3D_Vex2D* a_dest, X3D_Vex2D* b_dest, int16 z) {
  if(a->z < z && b->z < z) {
    return EDGE_INVISIBLE;
  }
  else if(a->z >= z && b->z >= z) {
    *a_dest = *a_project;
    *b_dest = *b_project;
    return 0;
  }
  
  if(a->z < z) {
    X3D_SWAP(a, b);
    X3D_SWAP(a_project, b_project);
    //SWAP(a_dest, b_dest);
  }
  
  *a_dest = *a_project;
  
  int16 dist_a = a->z - z;
  int16 dist_b = z - b->z;
  int16 scale = ((int32)dist_a << 15) / (dist_a + dist_b);
  
  printf("Clipped!\n");
  
  X3D_Vex3D new_b = {
    a->x + ((((int32)b->x - a->x) * scale) >> 15),
    a->y + ((((int32)b->y - a->y) * scale) >> 15),
    z
  };
  
  x3d_vex3d_int16_project(b_dest, &new_b);
  
//   static int16 count = 0;
//   
//   if(count++ == 1) {
//     x3d_error("{%d,%d,%d} - {%d,%d,%d} -> {%d,%d,%d} -> {%d,%d} - {%d,%d}",
//               a->x, a->y, a->z,
//               b->x, b->y, b->z,
//               new_b.x, new_b.y, new_b.z, a_dest->x, a_dest->y, b_dest->x, b_dest->y);
//   }
  
  return EDGE_NEAR_CLIPPED;
}

void bin_search(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region) {
  X3D_Vex2D mid;
  
  do {
    mid.x = (in.x + out.x) >> 1;
    mid.y = (in.y + out.y) >> 1;
    
    //x3d_log(X3D_INFO, "%d %d, %d %d - %d, %d\n", in.x, in.y, out.x, out.y, mid.x, mid.y);
    
    if(abs(in.x - out.x) < 2 && abs(in.y - out.y) < 2)
      break;
    
    
    if(clipregion_point_inside(region, mid)) {
      in = mid;
    }
    else {
      out = mid;
    }
  } while(1);
  
  res->x = mid.x;
  res->y = mid.y;
}

_Bool x3d_clipregion_clip_line(X3D_RasterRegion* region, X3D_Stack* stack, X3D_Vex2D* start, X3D_Vex2D* end) {
  // This is a terribly inefficient way to implement this...
  
  
  void* stack_ptr = x3d_stack_save(stack);
  X3D_RasterEdge edge;
  
  x3d_rasteredge_generate(stack, &edge, *start, *end, region->y_range);
  
  int16 y_min = X3D_MAX(region->y_range.min, edge.y_range.min);
  int16 y_max = X3D_MIN(region->y_range.max, edge.y_range.max);
  
  if(edge.flags & EDGE_INVISIBLE || y_min > y_max) {
    x3d_stack_restore(stack, stack_ptr);
    return X3D_FALSE;
  }
  
  // Horizontal lines
  if(start->y == end->y) {
    int16 y_offset = start->y - region->y_range.min;
    
    if(start->x > end->x) {
      X3D_SWAP(start, end);
    }
    
    if(start->x < region->x_left[y_offset])
      start->x = region->x_left[y_offset];
    
    if(end->x > region->x_right[y_offset])
      end->x = region->x_right[y_offset];
    
    x3d_stack_restore(stack, stack_ptr);
    
    return start->x < end->x;
    
  }
  
  int16 i;
  _Bool found = X3D_FALSE;
  int16 x, left, right;
  
  // Find where the line enters the region
  for(i = y_min; i <= y_max; ++i) {
    left = region->x_left[i - region->y_range.min];
    right = region->x_right[i - region->y_range.min];
    x = edge.x_data[i - edge.y_range.min];
    
    
    if(x >= left && x <= right) {
      found = X3D_TRUE;
      break;
    }
  }
  
  if(start->y > end->y) {
    X3D_SWAP(start, end);
  }
  
  if(i == y_min) {
    start->x = x;
    start->y = i;
  }
  else {
    X3D_Vex2D in = { x, i };
    X3D_Vex2D out = { edge.x_data[i - 1 - edge.y_range.min], i - 1 };
    
    bin_search(in, out, start, region);
  }
  
  if(!found) {
    x3d_stack_restore(stack, stack_ptr);
    return X3D_FALSE;
  }
  
  if(clipregion_point_inside(region, *end)) {
    x3d_stack_restore(stack, stack_ptr);
    return X3D_TRUE;
  }
  
  // Do a binary search to find the last pixel where the line is inside
  
  bin_search(*start, *end, end, region);
  
  //end->x = mid.x;
  //end->y = mid.y;
  
  x3d_stack_restore(stack, stack_ptr);
  
  return X3D_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
/// Fills a raster region.
///
/// @param region - region
/// @param color  - color to the fill the region with
///
/// @return Nothing.
/// @note No clipping is performed (it is assumed to already be clipped).
///////////////////////////////////////////////////////////////////////////////
void x3d_clipregion_fill(X3D_RasterRegion* region, X3D_Color color) {
  int16 i;
  
  for(i = region->y_range.min; i < region->y_range.max; ++i) {
    uint16 index = i - region->y_range.min;
    x3d_screen_draw_line(region->x_left[index], i, region->x_right[index], i, color);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the two endpoints of the rasteredge, after clipping.
///
/// @param edge   - edge
/// @param start  - start endpoint dest
/// @param end    - end endpoint dest
///
/// @return Nothing.
/// @note The endpoints are guaranteed to be in the same order as they were
///   passed into x3d_rasteredge_generate().
///////////////////////////////////////////////////////////////////////////////
void x3d_rasteredge_get_endpoints(X3D_RasterEdge* edge,  X3D_Vex2D* start, X3D_Vex2D* end) {
  *start = edge->start;
  *end = edge->end;
  
  #if 0
  x3d_assert(!(edge->flags & EDGE_INVISIBLE));
  
  if(edge->flags & EDGE_HORIZONTAL) {
    start->x = edge->x_range.min;
    start->y = edge->y_range.min;
    
    end->x = edge->x_range.max;
    end->y = edge->y_range.min;
  }
  else {
    start->x = edge->x_data[0];
    start->y = edge->y_range.min;
    
    end->x = edge->x_data[edge->y_range.max - edge->y_range.min];
    end->y = edge->y_range.max;
  }
  
  if(edge->flags & EDGE_V_SWAPPED)
    X3D_SWAP(*start, *end);
#endif
}


#endif