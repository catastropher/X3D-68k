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
#include "X3D_clip.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"
#include "memory/X3D_stack.h"
#include "X3D_assert.h"
#include "X3D_keys.h"

int16 render_mode = 3;

// This code is absolutely terrible and badly needs to be refactored... it was
// never intended to *actually* be used... annoy Michael until he refactors
// this mess.


#define LCD_WIDTH x3d_screenmanager_get()->w
#define LCD_HEIGHT x3d_screenmanager_get()->h

//#undef printf
//#define printf(...) ;

#define x3d_error(args...) x3d_log(X3D_ERROR, args);

#ifndef INT16_MIN
#define INT16_MIN -32767
#endif







fp16x16 x3d_vertical_slope(X3D_Vex2D v1, X3D_Vex2D v2) {
  if(v1.y == v2.y)
    return 0;
  
  return (((int32)(v2.x - v1.x)) << 16) / (v2.y - v1.y);
}

///////////////////////////////////////////////////////////////////////////////
/// Finds the intersection point between a line and a horizontal line.
///
/// @param slope  - slope of the line
/// @param start  - starting position of the line
/// @param y      - y position of the horizontal line
///
/// @return Nothing.
/// @note The intersection point is written back to start.
///////////////////////////////////////////////////////////////////////////////
void x3d_intersect_line_with_horizontal(fp16x16 slope, X3D_Vex2D* start, int16 y) {
  //x3d_assert((slope >> 16) < 128);    // To prevent overflow when converting to fp8x8 for the slope
  
  int16 dy = y - start->y;
  int16 slope_8x8 = slope >> 8;

  /// @todo Optomize to not use 64 bit multiplication (maybe using a binary search?)
#if !defined(__68k__)
  start->x = start->x + (((int64)dy * slope) >> 16);
  start->y = y;
#else
  start->x = start->x + (((int32)dy * slope_8x8) >> 8);
  start->y = y;
  
#endif
}


#define EDGE_VALUE(_edge, _y) ((_edge)->x_data[_y - (_edge)->rect.y_range.min])

#define EDGE(_edge) raster_edge[edge_index[_edge]]

#define REGION_OFFSET(_region, _y) (_y - _region->min_y)

///////////////////////////////////////////////////////////////////////////////
/// Constructs a raster region from a list of edges.
///
/// @param region       - dest region
/// @param stack        - stack to allocate the region x_data on
/// @param raster_edge  - an array of raster edges
/// @param edge_index   - a list of edges to select from raster_edge to
///     construct the region
/// @param total_e      - number of edges in edge_index
///
/// @return Whether a potentially visible raster region has been constructed
/// @todo   This function can be very easily optimized!
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_rasterregion_construct_from_edges(X3D_RasterRegion* region, X3D_RasterRegion* parent, X3D_Stack* stack, X3D_RasterEdge raster_edge[], int16 edge_index[], int16 total_e) {
  X3D_Range* y_range = &region->rect.y_range;
  
  // Initially, set each scanline to have invalid values. They will get replaced
  // as each edge is filled in
  int16 i;
  
  y_range->min = INT16_MAX;
  y_range->max = INT16_MIN;
  
  for(i = 0; i < total_e; ++i) {
    if(!x3d_rasteredge_invisible(&EDGE(i))) {
      y_range->min = X3D_MIN(y_range->min, EDGE(i).rect.y_range.min);
      y_range->max = X3D_MAX(y_range->max, EDGE(i).rect.y_range.max);
    }
    else if(!x3d_rasteredge_frustum_clipped(&EDGE(i))) {
      y_range->min = X3D_MIN(y_range->min, parent->rect.y_range.min);
    }
  }
  
  //x3d_log(X3D_INFO, "Range: %d-%d\n", y_range->min, y_range->max);
  
  region->span = x3d_stack_alloc(stack, sizeof(X3D_Span) * (y_range->max - y_range->min + 1));
  
  for(i = y_range->min; i <= y_range->max; ++i) {
    region->span[i - y_range->min].old_left_val = INT16_MAX;
    region->span[i - y_range->min].old_right_val = INT16_MIN;
  }
  
  X3D_RasterEdge* e;
  
  
  // Add each edge to the raster region
  int16 edge;
  for(edge = 0; edge < total_e; ++edge) {
    e = &EDGE(edge);
    
    if(!x3d_rasteredge_invisible(e)) {
      X3D_Span* span = region->span + e->rect.y_range.min - y_range->min;
      
      if(x3d_rasteredge_horizontal(e)) {
        // Just replace the left/right values of the horizontal line, if necessary
        if(e->rect.x_range.min < span->old_left_val) {
	  span->old_left_val = e->rect.x_range.min;
	}
	  
	if(e->rect.x_range.max > span->old_right_val) {
	  span->old_right_val = e->rect.x_range.max;
	}
      }
      else {
        int16 i;
        int16* x = e->x_data;
	if(e->rect.y_range.max != e->rect.y_range.min) {
	
	  // Calculate the scale interpolation slope
    //x3d_assert(e->start_scale >= 0 && e->end_scale >= 0);
    
    
	  
	  // For each x value in the edge, check if the x_left and x_right values
	  // in the raster region need to be replaced by it
	  for(i = e->rect.y_range.min; i <= e->rect.y_range.max; ++i) {
	    if(*x < span->old_left_val) {
	      span->old_left_val = *x;
	    }
	    
	    if(*x > span->old_right_val) {
	      span->old_right_val = *x;
	    }
	    
	    ++x;
	    ++span;
	  }
	}
      }      
    }
  }
  
  x3d_assert(((size_t)region->span & 1) == 0);
  
  // A region is only valid if the region's min_y <= max_y
  if(region->rect.y_range.min <= region->rect.y_range.max) {
    //region->span += region->rect.y_range.min;
    return X3D_TRUE;
  }
  
  return X3D_FALSE;
}

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
_Bool x3d_rasterregion_construct_from_points(X3D_Stack* stack, X3D_RasterRegion* dest, X3D_Vex2D* v, uint16 total_v) {
  X3D_RasterEdge edges[total_v];
  uint16 edge_index[total_v];
  uint16 i;
  
  // Create a fake parent region that takes up the whole screen
  X3D_RasterRegion fake_parent_region = {
    .rect = {
      .y_range = {
        0, x3d_screenmanager_get()->h - 1
      }
    }
  };
  
  for(i = 0; i < total_v; ++i) {
    int16 next = (i + 1) % total_v;
    x3d_rasteredge_generate(edges + i, v[i], v[next], &fake_parent_region, 100, 100, stack, 0x7FFF / 2, 0x7FFF / 2);
    edge_index[i] = i;
  }
  
  return x3d_rasterregion_construct_from_edges(dest, (dest != &x3d_rendermanager_get()->region ? &x3d_rendermanager_get()->region : NULL), stack, edges, edge_index, total_v);
}

#define CLIP() x3d_span_clip(region_span, *parent_span)

///////////////////////////////////////////////////////////////////////////////
/// Clips a single span against another span (from the parent region)
///
/// @param span         - span to clip
/// @param parent_span  - span in the parent region to clip against
///
/// @return Whether a valid span remains after clipping.
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_span_clip(X3D_Span* span, X3D_Span parent_span) {
  //span->left =  X3D_MAX(parent_span.left, span->left);
  //span->right = X3D_MIN(parent_span.right, span->right);
  
  
  
  if(span->old_left_val < parent_span.old_left_val) {
    int32 left = abs(span->old_left_val - parent_span.old_left_val);
    int32 right = abs(span->old_right_val - parent_span.old_left_val);
    
    span->old_left_val = parent_span.old_left_val;
  }
  
  
#if 1
  if(span->old_right_val > parent_span.old_right_val) {
    int32 left = abs(span->old_left_val - parent_span.old_right_val);
    int32 right = abs(span->old_right_val - parent_span.old_right_val);
    
    span->old_right_val = parent_span.old_right_val;
  }
  
#endif
  
  
  return span->old_left_val <= span->old_right_val;
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the intersection between two raster regions.
///
/// @param region - first region to intersect (result written here)
/// @param parent - the second region to intersect (the "parent" region)
///
/// @return Whether a valid region is produced after doing the intersection
/// @todo   This function really needs to be optimized.
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_rasterregion_intersect(X3D_RasterRegion* region, X3D_RasterRegion* parent) {
  X3D_Span* parent_span = parent->span + region->rect.y_range.min - parent->rect.y_range.min;
  X3D_Span* region_span = region->span;
  
  int16 y = region->rect.y_range.min;

  // Check to make sure we have valid y ranges
  x3d_assert(y >= 0 && y < x3d_screenmanager_get()->h);  
  x3d_assert_range(y, region->rect.y_range.min, region->rect.y_range.max + 1);
  
  // Skip fully clipped spans
  while(y <= region->rect.y_range.max && !CLIP()) {
    ++y;
    ++region_span;
    ++parent_span;
  }
  
  // If all the spans are fully clipped, it's invisible!
  if(y > region->rect.y_range.max) {
    return X3D_FALSE;
  }
  
  // We need to actually adjust x_left and x_right to point to the first visible span
  region->span = region_span;
  
  region->rect.y_range.min = y;
  
  while(y <= region->rect.y_range.max && CLIP()) {
    ++y;
    ++parent_span;
    ++region_span;
  }
  
  region->rect.y_range.max = y - 1;
  
  return X3D_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
/// Determines whether a point is inside a raster region.
///
/// @param region - region
/// @param p      - point
///
/// @return Whether the point is inside the region.
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_rasterregion_point_inside(X3D_RasterRegion* region, X3D_Vex2D p) {
  if(p.y < region->rect.y_range.min || p.y > region->rect.y_range.max)
    return X3D_FALSE;
  
  uint16 offsety = p.y - region->rect.y_range.min;
  
  return p.x >= region->span[offsety].old_left_val && p.x <= region->span[offsety].old_right_val;
}


int16 x3d_clip_line_to_near_plane(X3D_Vex3D* a, X3D_Vex3D* b, X3D_Vex2D* a_project, X3D_Vex2D* b_project, X3D_Vex2D* a_dest, X3D_Vex2D* b_dest, int16 z) {
  uint16 flags = 0;
  
  // Quick rejection against pseudo view frustum planes
  
 //  if(!x3d_key_down(X3D_KEY_15)) {
  // Bottom plane
  
  //if(!x3d_key_down(X3D_KEY_15)) {

    
#if 1
 
  // Left
  if(a->x < -a->z && b->x < -b->z) {
    flags |= EDGE_LEFT_CLIPPED;
  }

  // Right
  if(a->x > a->z && b->x > b->z) {
    flags |= EDGE_RIGHT_CLIPPED;
  }
  
  if(a->y > a->z / 2 && b->y > b->z / 2) {
    //printf("INVISIBLE!!!\n");
    //return EDGE_INVISIBLE;
    flags |= EDGE_BOTTOM_CLIPPED;
  }
  
  // Top plane
  if(a->y < -a->z / 2 && b->y < -b->z / 2) {
    //printf("INVISIBLE!!!\n");
    //return EDGE_INVISIBLE;
    flags |= EDGE_TOP_CLIPPED;
  }

  
  if(a->z < z && b->z < z) {
    flags |= EDGE_NEAR_CLIPPED | EDGE_INVISIBLE;
    return flags;
  }
  
#endif

  
   //}
  
  
//  }
  
  if(a->z >= z && b->z >= z) {
    *a_dest = *a_project;
    *b_dest = *b_project;
    return flags;
  }
  
  // Check against the pseudo left plane
  
  
  if(a->z < z) {
    X3D_SWAP(a, b);
    X3D_SWAP(a_project, b_project);
    X3D_SWAP(a_dest, b_dest);
  }
  
  *a_dest = *a_project;
  
  int16 dist_a = a->z - z;
  int16 dist_b = z - b->z;
  int16 scale = ((int32)dist_a << 15) / (dist_a + dist_b);
  
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
  
  return flags;//EDGE_NEAR_CLIPPED;
}

void bin_search(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region) {
  X3D_Vex2D mid;
  
  do {
    mid.x = (in.x + out.x) >> 1;
    mid.y = (in.y + out.y) >> 1;
    
    //x3d_log(X3D_INFO, "%d %d, %d %d - %d, %d\n", in.x, in.y, out.x, out.y, mid.x, mid.y);
    
    if(abs(in.x - out.x) < 2 && abs(in.y - out.y) < 2)
      break;
    
    
    if(x3d_rasterregion_point_inside(region, mid)) {
      in = mid;
    }
    else {
      out = mid;
    }
  } while(1);
  
  res->x = mid.x;
  res->y = mid.y;
}

_Bool x3d_rasterregion_clip_line(X3D_RasterRegion* region, X3D_Stack* stack, X3D_Vex2D* start, X3D_Vex2D* end) {
  // This is a terribly inefficient way to implement this...
  
  
  void* stack_ptr = x3d_stack_save(stack);
  X3D_RasterEdge edge;

#if 0
  if(start->y > end->y) {
    X3D_SWAP(start, end);    
  }
  
  if(start->y < 0 && end->y > 0) {
    while(start->y < -5000) {
      start->x = (start->x + end->x) / 2;
      start->y = (start->y + end->y) / 2;
    }
    
    while(end->y > 5000) {
      end->x = (start->x + end->x) / 2;
      end->y = (start->y + end->y) / 2;
    }
  }
#endif
  
  x3d_rasteredge_generate(&edge, *start, *end, region, 0, 0, stack, 0x7FFF, 0x7FFF);
  
  int32 y_min = X3D_MAX(region->rect.y_range.min, edge.rect.y_range.min);
  int32 y_max = X3D_MIN(region->rect.y_range.max, edge.rect.y_range.max);
  
  if((edge.flags & EDGE_INVISIBLE) || y_min > y_max) {
    x3d_stack_restore(stack, stack_ptr);
    return X3D_FALSE;
  }
  
  // Horizontal lines
  if(start->y == end->y) {
    int16 y_offset = start->y - region->rect.y_range.min;
    
    if(start->x > end->x) {
      X3D_SWAP(start, end);
    }
    
    if(start->x < region->span[y_offset].old_left_val)
      start->x = region->span[y_offset].old_left_val;
    
    if(end->x > region->span[y_offset].old_right_val)
      end->x = region->span[y_offset].old_right_val;
    
    x3d_stack_restore(stack, stack_ptr);
    
    return start->x < end->x;
    
  }
  
#if 0
  start->x = edge.x_data[0];
  start->y = edge.rect.y_range.min;
  
  end->x = edge.x_data[edge.rect.y_range.max - edge.rect.y_range.min];
  end->y = edge.rect.y_range.max;
#endif
  
  int32 i;
  _Bool found = X3D_FALSE;
  int16 x, left, right;

  // Find where the line enters the region
  for(i = y_min; i <= y_max; ++i) {
    left = region->span[i - region->rect.y_range.min].old_left_val;
    right = region->span[i - region->rect.y_range.min].old_right_val;
    x = edge.x_data[i - edge.rect.y_range.min];
    
    
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
    X3D_Vex2D out = { edge.x_data[i - 1 - edge.rect.y_range.min], i - 1 };
    
    bin_search(in, out, start, region);
  }
  
  if(!found) {
    x3d_stack_restore(stack, stack_ptr);
    return X3D_FALSE;
  }
  
  if(x3d_rasterregion_point_inside(region, *end)) {
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

uint32 x3d_color_to_internal(X3D_Color c);

///////////////////////////////////////////////////////////////////////////////
/// Fills a raster region.
///
/// @param region - region
/// @param color  - color to the fill the region with
///
/// @return Nothing.
/// @note No clipping is performed (it is assumed to already be clipped).
///////////////////////////////////////////////////////////////////////////////
void x3d_rasterregion_fill(X3D_RasterRegion* region, X3D_Color color) {
  int16 i;

  uint16 total_c = 64;
  
  X3D_Color color_tab[total_c + 10];
  
  uint8 rr, gg, bb;
  x3d_color_to_rgb(color, &rr, &gg, &bb);
  
  uint32 r_slope = ((uint16)rr << 16) / (total_c + 1);
  uint32 g_slope = ((uint16)gg << 16) / (total_c + 1);
  uint32 b_slope = ((uint16)bb << 16) / (total_c + 1);
  
  uint32 r = 0, g = 0, b = 0;
 
#if 0
  for(i = 0; i < 5; ++i) {
#ifdef __nspire__
    color_tab[4 - i] = x3d_color_to_internal(x3d_rgb_to_color((r_slope / (i + 1)) >> 16, (g_slope / (i + 1)) >> 16, (b_slope / (i + 1)) >> 16));
#else
    color_tab[4 - i] = x3d_rgb_to_color((r_slope / (i + 1)) >> 16, (g_slope / (i + 1)) >> 16, (b_slope / (i + 1)) >> 16);
#endif
  }
#endif
  
  for(i = 0; i < total_c; ++i) {
#ifdef __nspire__
    color_tab[i + 5] = x3d_color_to_internal(x3d_rgb_to_color(r >> 16, g >> 16, b >> 16));
#else
    color_tab[i + 5] = x3d_rgb_to_color(r >> 16, g >> 16, b >> 16);
#endif
    r += r_slope;
    g += g_slope;
    b += b_slope;
  }
  
  for(i = 0; i < 5; ++i) {
    color_tab[i] = 0;
    color_tab[i + total_c + 5] = color_tab[total_c - 1];
  }
  
  for(i = region->rect.y_range.min; i < region->rect.y_range.max; ++i) {
    uint16 index = i - region->rect.y_range.min;
    
    //X3D_Color color_left = x3d_color_scale(color, region->span[index].left_scale);
    //X3D_Color color_right = x3d_color_scale(color, region->span[index].right_scale);

    //x3d_screen_draw_scanline_grad(i, region->span[index].old_left_val, region->span[index].old_right_val, color, region->span[index].left_scale, region->span[index].right_scale, color_tab + 5, 0);
    
    //x3d_screen_draw_line_grad(region->span[index].left, i, region->span[index].right, i, color_left, color_right);
  }
}

//=============================================

///////////////////////////////////////////////////////////////////////////////
/// Fills a raster region.
///
/// @param region - region
/// @param color  - color to the fill the region with
///
/// @return Nothing.
/// @note No clipping is performed (it is assumed to already be clipped).
///////////////////////////////////////////////////////////////////////////////
void x3d_rasterregion_fill_zbuf(X3D_RasterRegion* region, X3D_Color color, int16 z) {
  int16 i;

  uint16 total_c = 64;
  
  X3D_Color color_tab[total_c + 10];
  
  uint8 rr, gg, bb;
  x3d_color_to_rgb(color, &rr, &gg, &bb);
  
  uint32 r_slope = ((uint16)rr << 16) / (total_c + 1);
  uint32 g_slope = ((uint16)gg << 16) / (total_c + 1);
  uint32 b_slope = ((uint16)bb << 16) / (total_c + 1);
  
  uint32 r = 0, g = 0, b = 0;
 
#if 0
  for(i = 0; i < 5; ++i) {
#ifdef __nspire__
    color_tab[4 - i] = x3d_color_to_internal(x3d_rgb_to_color((r_slope / (i + 1)) >> 16, (g_slope / (i + 1)) >> 16, (b_slope / (i + 1)) >> 16));
#else
    color_tab[4 - i] = x3d_rgb_to_color((r_slope / (i + 1)) >> 16, (g_slope / (i + 1)) >> 16, (b_slope / (i + 1)) >> 16);
#endif
  }
#endif
  
  for(i = 0; i < total_c; ++i) {
#ifdef __nspire__
    color_tab[i + 5] = x3d_color_to_internal(x3d_rgb_to_color(r >> 16, g >> 16, b >> 16));
#else
    color_tab[i + 5] = x3d_rgb_to_color(r >> 16, g >> 16, b >> 16);
#endif
    r += r_slope;
    g += g_slope;
    b += b_slope;
  }
  
  for(i = 0; i < 5; ++i) {
    color_tab[i] = 0;
    color_tab[i + total_c + 5] = color_tab[total_c - 1];
  }
  
  for(i = region->rect.y_range.min; i < region->rect.y_range.max; ++i) {
    uint16 index = i - region->rect.y_range.min;
    
    //X3D_Color color_left = x3d_color_scale(color, region->span[index].left_scale);
    //X3D_Color color_right = x3d_color_scale(color, region->span[index].right_scale);

    //x3d_screen_draw_scanline_grad(i, region->span[index].old_left_val, region->span[index].old_right_val, color, region->span[index].left_scale, region->span[index].right_scale, color_tab + 5, z);
    
    //x3d_screen_draw_line_grad(region->span[index].left, i, region->span[index].right, i, color_left, color_right);
  }
}

void x3d_rasterregion_fill_texture(X3D_RasterRegion* r, int16 z) {
  uint16 i;
  
  for(i = r->rect.y_range.min; i <= r->rect.y_range.max; ++i) {
    x3d_screen_draw_scanline_texture(x3d_rasterregion_get_span(r, i), i);
  }
}

//=============================================

fp0x16 x3d_point_intensity(X3D_Segment* seg, uint16 p, X3D_Vex3D* face_normal, int16 z) {
  if(face_normal) {
    X3D_Vex3D normal;
    x3d_segment_point_normal(seg, p, &normal, face_normal, ANG_90 - 5);
      

    X3D_Vex3D d = { 0, 0, 32767 };
    fp0x16 dot = abs(x3d_vex3d_fp0x16_dot(&d, &normal));
    
    dot = X3D_MIN((int32)dot + 8192, 32767);

    int16 val = ((int32)x3d_depth_scale(z, 10, 1500) * dot) >> 15;
    
    x3d_assert(val >= 0);
    
    return val;
  }
  
  return NULL;
}

/// Under construction
/// @todo Refactor!!!!!!!!
_Bool x3d_rasterregion_construct_clipped(X3D_ClipContext* clip, X3D_RasterRegion* dest) {
  uint16 total_e = clip->total_e;
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  uint16 i;
  uint16 total_vis_e = 0;   // How many edges are actually visible
  int16 near_z = x3d_rendermanager_get()->near_z;
  X3D_Vex2D out_v[2];
  uint16 total_out_v = 0;
  uint16 vis_e[clip->total_edge_index + 5];
  _Bool close = X3D_FALSE;
  int16 depth[2];
  
  _Bool left_clipped = X3D_FALSE;
  _Bool right_clipped = X3D_FALSE;
  _Bool top_clipped = X3D_FALSE;
  _Bool bottom_clipped = X3D_FALSE;
  


  uint16 flags = EDGE_NEAR_CLIPPED | EDGE_LEFT_CLIPPED | EDGE_RIGHT_CLIPPED | EDGE_TOP_CLIPPED | EDGE_BOTTOM_CLIPPED;

  
  for(i = 0; i < clip->total_edge_index; ++i) {
    X3D_Pair edge = clip->edge_pairs[clip->edge_index[i]];
    uint16 in[2] = { clip->v3d[edge.val[0]].z >= near_z, clip->v3d[edge.val[1]].z >= near_z };

    int16 d;
    
    flags &= clip->edges[clip->edge_index[i]].flags;
    
    //printf("Flags: %d\n", flags);
    
    
    _Bool frustum_clipped = X3D_FALSE;

#if 0    
    if((clip->edges[clip->edge_index[i]].flags & EDGE_NEAR_CLIPPED))
      frustum_clipped = X3D_TRUE;

    if((clip->edges[clip->edge_index[i]].flags & EDGE_LEFT_CLIPPED)) {
      frustum_clipped = X3D_TRUE;
      left_clipped = X3D_TRUE;
    }
    
    if((clip->edges[clip->edge_index[i]].flags & EDGE_RIGHT_CLIPPED)) {
      frustum_clipped = X3D_TRUE;
      right_clipped = X3D_TRUE;
    }
    
    if((clip->edges[clip->edge_index[i]].flags & EDGE_TOP_CLIPPED)) {
      frustum_clipped = X3D_TRUE;
      top_clipped = X3D_TRUE;
    }
    
    if((clip->edges[clip->edge_index[i]].flags & EDGE_BOTTOM_CLIPPED)) {
      frustum_clipped = X3D_TRUE;
      bottom_clipped = X3D_TRUE;
    }
#endif
    
    
    
    if((in[0] || in[1]) && !frustum_clipped) {
      vis_e[total_vis_e++] = clip->edge_index[i];
      
      if(render_mode == 3) {
        X3D_Pair p = clip->edge_pairs[clip->edge_index[i]];
        int16 ia = x3d_point_intensity(clip->seg, p.val[0], clip->normal, clip->v3d[p.val[0]].z);
        int16 ib = x3d_point_intensity(clip->seg, p.val[1], clip->normal, clip->v3d[p.val[1]].z);
        x3d_rasteredge_set_intensity(&clip->edges[clip->edge_index[i]], ia, ib);
      }
      
      if(in[0] != in[1]) {
        X3D_Vex2D v[2];
        
        x3d_rasteredge_get_endpoints(clip->edges + clip->edge_index[i], v, v + 1);        
        
        // FIXME
        if(in[0]) {
          //printf("OUT!!!\n");
          out_v[total_out_v] = v[1];
          depth[total_out_v++] = clip->edges[clip->edge_index[i]].start.z;
        }
        else {
          //printf("OUT!!!\n");
          out_v[total_out_v] = v[0];
          depth[total_out_v++] = clip->edges[clip->edge_index[i]].end.z;
        }
          
        //out_v[total_out_v++] = v[in[0]];
        
        //clip->edge_pairs[clip->edge_index[i]].val[in[0]];
      }
    }
    else {
      //close = close || clip->v3d[edge.val[0]].z >= 0 || clip->v3d[edge.val[1]].z >= 0;
    }
  }
  
  if(flags) {
#if 0
    printf("fail flags: ");
    
    printf("%c", (flags & EDGE_NEAR_CLIPPED) ? 'N' : ' ');
    printf("%c", (flags & EDGE_LEFT_CLIPPED) ? 'L' : ' ');
    printf("%c", (flags & EDGE_RIGHT_CLIPPED) ? 'R' : ' ');
    printf("%c", (flags & EDGE_TOP_CLIPPED) ? 'T' : ' ');
    printf("%c", (flags & EDGE_BOTTOM_CLIPPED) ? 'B' : ' ');
    printf("%c", (flags & EDGE_INVISIBLE) ? 'I' : ' ');

    printf("\n");
#endif
    
    for(i = 0; i < clip->total_edge_index; ++i) {
      clip->edges[clip->edge_index[i]].flags |= EDGE_NO_DRAW;
    }
    
    return X3D_FALSE;
  }
  
  //x3d_assert(total_out_v <= 2);
    
  // Create a two edge between the two points clipped by the near plane
  if(total_out_v == 2) { 
    /// FIXME please!
#if 1
    x3d_rasteredge_generate(clip->edges + total_e,
      out_v[0], out_v[1], clip->parent, depth[0], depth[1], &renderman->stack, 0x7FFF, 0x7FFF);
    
    
    //printf("Line: {%d,%d} - {%d,%d}, %d\n", out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y, total_vis_e + 1);
    
    //x3d_screen_draw_line(out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y, 0);
    
    
    vis_e[total_vis_e++] = total_e++;
#endif
  }
  
#if 1
  if(top_clipped) {
    X3D_Vex2D a = { 0, 0 };
    X3D_Vex2D b = { x3d_screenmanager_get()->w - 1, 0 };
    
    x3d_log(X3D_INFO, "Loc!");
    
    x3d_rasteredge_generate(clip->edges + total_e,
      a, b, clip->parent, 10, 10, &renderman->stack, 0x7FFF, 0x7FFF);
    
    vis_e[total_vis_e++] = total_e++;
  }
  
  if(bottom_clipped) {
    X3D_Vex2D a = { 0, x3d_screenmanager_get()->h - 1 };
    X3D_Vex2D b = { x3d_screenmanager_get()->w - 1, x3d_screenmanager_get()->h - 1 };
    
    x3d_rasteredge_generate(clip->edges + total_e,
      a, b, clip->parent, 10, 10, &renderman->stack, 0x7FFF, 0x7FFF);
    
    vis_e[total_vis_e++] = total_e++;
  }
#endif
  
  //printf("Total vis e: %d\nOut v: %d\n", total_vis_e, total_out_v);
  
  //return total_vis_e > 0 &&
  if(1) {
    if(x3d_rasterregion_construct_from_edges(dest, clip->parent, &renderman->stack, clip->edges, vis_e, total_vis_e)) {
      if(total_out_v == 2 && x3d_rasterregion_clip_line(clip->parent, &renderman->stack, out_v, out_v + 1)) {
        uint16 i;
    
        printf("HERE\n");
        
        int16 y_index = out_v[0].y - dest->rect.y_range.min;
        
        //x3d_assert(y_index >= dest->y_range.min);
        
        if(y_index < dest->rect.y_range.min)
          y_index = 0;
        
        if(abs(out_v[0].x - dest->span[y_index].old_left_val) < abs(out_v[0].x - dest->span[y_index].old_right_val)) {
          printf("Left!\n");
          
          if(!x3d_key_down(X3D_KEY_15)) {
            for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
              dest->span[i].old_left_val = 0;
          }
        }
        else {
          if(!x3d_key_down(X3D_KEY_15)) {
            for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
              dest->span[i].old_right_val = x3d_screenmanager_get()->w - 1;
          }
          
          printf("Right!\n");
        }
      }
      
      
      if(left_clipped) {
        for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
          dest->span[i].old_left_val = 0;
      }
      
      if(right_clipped) {
        for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
          dest->span[i].old_right_val = x3d_screenmanager_get()->w - 1;
      }
      
#if 0
      if(right_clipped && x3d_key_down(X3D_KEY_15)) {
        X3D_Vex2D a = { x3d_screenmanager_get()->w - 1, 0 };
        X3D_Vex2D b = { x3d_screenmanager_get()->w - 1, x3d_screenmanager_get()->h - 1 };
        
        x3d_rasteredge_generate(clip->edges + total_e,
          a, b, clip->parent, 10, 10, &renderman->stack);
        
        vis_e[total_vis_e++] = total_e++;
      }
#endif
      
      if(x3d_rasterregion_intersect(dest, clip->parent)) {
        X3D_Color gray = x3d_rgb_to_color(64, 64, 64);
        //x3d_rasterregion_fill(dest, gray);
        X3D_Color color = x3d_rgb_to_color(255, 255, 255);
        //x3d_screen_draw_line(out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y, color);
        return X3D_TRUE;
      }
    }
  }
  
  clip->really_close = close;
  
  return X3D_FALSE;
}



