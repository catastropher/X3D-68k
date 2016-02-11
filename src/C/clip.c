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

///////////////////////////////////////////////////////////////////////////////
/// Checks if an edge is horizontal; if so, it sets the X3D_EDGE_HORIZONTAL
///     flag.
///
/// @param edge - edge
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_rasteredge_set_horizontal_flag(X3D_RasterEdge* edge) {
  if(edge->rect.y_range.min == edge->rect.y_range.max) {
    edge->flags |= EDGE_HORIZONTAL;
  }
}

void x3d_rasteredge_set_invisible_flag(X3D_RasterEdge* edge, X3D_Range region_y_range) {
  if(!range_overlap(edge->rect.y_range, region_y_range)) {
    edge->flags |= EDGE_INVISIBLE;
  }
}

_Bool x3d_rasteredge_invisible(X3D_RasterEdge* edge) {
  return edge->flags & EDGE_INVISIBLE;
}

_Bool x3d_rasteredge_horizontal(X3D_RasterEdge* edge) {
  return edge->flags & EDGE_HORIZONTAL;
}

void x3d_rasteredge_set_y_range(X3D_RasterEdge* edge, X3D_Vex2D* a, X3D_Vex2D* b) {
  edge->rect.y_range = get_range(a->y, b->y);
}

void x3d_rasteredge_set_x_range(X3D_RasterEdge* edge, X3D_Vex2D* a, X3D_Vex2D* b) {
  edge->rect.x_range = get_range(a->x, b->x);
}

///////////////////////////////////////////////////////////////////////////////
/// Vertically clips a raster edge against the parent raster region and
///   calculates the slope of the edge.
///
/// @param edge           - edge to clip
/// @param a              - start vertex
/// @param b              - end vertex
/// @param slope          - slope dest
/// @param parent_y_range - vertical range of the parent raster region
///
/// @return Whether the edge is at least potentially visible.
/// @note a.y must be <= b.y
/// @note After clipping, the resulting vertices will be stored back into a and
///   b.
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_rasteredge_clip(X3D_RasterEdge* edge, X3D_Vex2D* a, X3D_Vex2D* b, fp16x16* slope, X3D_Range parent_y_range) {
  // This clips the raster edge against the line y=region_y_range.min.
  // That way, we don't waste effort calculating values of the edge that are
  // invisible. But, we only want to do this if 1) the line isn't invisible
  // and 2) if the edge isn't horizontal (if the edge is visible and is horizontal,
  // it can't possibly need to be clipped)
  
  if(x3d_rasteredge_invisible(edge))
    return X3D_FALSE;
  
  if(!x3d_rasteredge_horizontal(edge)) {
    // Calculate the slope of the edge
    *slope = x3d_vertical_slope(*a, *b);
    
    // Clip the edge, if necessary
    if(a->y < parent_y_range.min) {
      x3d_intersect_line_with_horizontal(*slope, a, parent_y_range.min);
      
      // Update the minumum y of the edge, since it was just clipped
      edge->rect.y_range.min = parent_y_range.min;
    }
    
    // Clamp the max y of the edge, which is either the max y of the region or the max
    // y of the edge (it should be the minumum of the two)
    edge->rect.y_range.max = b->y = X3D_MIN(edge->rect.y_range.max, parent_y_range.max);
  }
  
  return X3D_TRUE;
}

#define EDGE_VALUE(_edge, _y) ((_edge)->x_data[_y - (_edge)->rect.y_range.min])

///////////////////////////////////////////////////////////////////////////////
/// Generates a raster edge from two endpoints.
///
/// @param edge     - edge
/// @param a        - first vertex
/// @param b        - second vertex
/// @param parent   - parent clipping region that the edge is inside of
/// @param depth_a  - depth of the first vertex (after transformed to be
///     relative to the camera)
/// @param depth_b  - depth of the second vertex (after transformed to be)
///     relative to the camera
/// @param stack    - stack to allocate x values on
///////////////////////////////////////////////////////////////////////////////
void x3d_rasteredge_generate(X3D_RasterEdge* edge, X3D_Vex2D a, X3D_Vex2D b, X3D_RasterRegion* parent, int16 depth_a, int16 depth_b, X3D_Stack* stack) {
  // Set start/end endpoints before clipping, as we want the original points.
  // Note that the Z component is the depth of the point after it was transformed
  // relative to the camera. It's not used directly by anything here, but is useful
  // for e.g. calculating the color of the edge based on its depth.
  edge->start.x = a.x;
  edge->start.y = a.y;
  edge->start.z = depth_a;
  
  edge->end.x = b.x;
  edge->end.y = b.y;
  edge->end.z = depth_b;
  
  edge->flags = 0;
  edge->x_data = NULL;
  
  // Swap points if out of order vertically
  if(a.y > b.y)
    X3D_SWAP(a, b);
  
  // Set some flags and the y range for the edge
  x3d_rasteredge_set_y_range(edge, &a, &b);
  x3d_rasteredge_set_horizontal_flag(edge);
  x3d_rasteredge_set_invisible_flag(edge, parent->rect.y_range);
  
  // Check to make sure we have a valid "parent" raster region y range (a valid parent
  // region never has its y range off screen and its y_range.min <= y_range.max)
  x3d_assert(parent->rect.y_range.min >= 0 && parent->rect.y_range.max < x3d_screenmanager_get()->h);
  x3d_assert(parent->rect.y_range.min <= parent->rect.y_range.max);
  
  fp16x16 slope;    // Slope of the edge
  
  // Only generate the edge if it's (potentially) visible
  if(x3d_rasteredge_clip(edge, &a, &b, &slope, parent->rect.y_range)) {
    fp16x16 x = ((int32)a.x) * 65536L;      // The top x position of the line
    int16 y = a.y;                          // Current y position
    int16 height = b.y - a.y + 1;           // Height of the edge
    
    // More checks to prevent invalid clipping
    x3d_assert(in_range(SCREEN_Y_RANGE, height - 1));
    x3d_assert(in_range(parent->rect.y_range, a.y));
    x3d_assert(in_range(parent->rect.y_range, b.y));
    
    // Allocate space for the values
    edge->x_data = x3d_stack_alloc(stack, height * 2);

    // For each y, generate an x value for the edge
    do {
      EDGE_VALUE(edge, y) = x >> 16;
      x += slope;
      ++y;
    } while(y <= b.y);
    
    b.x = (x - slope) >> 16;
  }

  // The x range has to be set after clipping because b.x may have been updated
  // by the clip (b hold either the real end of the edge or where it exits the
  // region vertically)
  x3d_rasteredge_set_x_range(edge, &a, &b);
}

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
_Bool x3d_rasterregion_construct_from_edges(X3D_RasterRegion* region, X3D_Stack* stack, X3D_RasterEdge raster_edge[], int16 edge_index[], int16 total_e) {
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
  }
  
  //x3d_log(X3D_INFO, "Range: %d-%d\n", y_range->min, y_range->max);
  
  region->span = x3d_stack_alloc(stack, sizeof(X3D_Span) * (y_range->max - y_range->min + 1));
  
  for(i = y_range->min; i <= y_range->max; ++i) {
    region->span[i - y_range->min].left = INT16_MAX;
    region->span[i - y_range->min].right = INT16_MIN;
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
        if(e->rect.x_range.min < span->left)    span->left = e->rect.x_range.min;
        if(e->rect.x_range.max > span->right)   span->right = e->rect.x_range.max;
      }
      else {
        int16 i;
        int16* x = e->x_data;
        
        // For each x value in the edge, check if the x_left and x_right values
        // in the raster region need to be replaced by it
        for(i = e->rect.y_range.min; i <= e->rect.y_range.max; ++i) {
          if(*x < span->left)    span->left = *x;
          if(*x > span->right)   span->right = *x;
          
          ++x;
          ++span;
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
    x3d_rasteredge_generate(edges + i, v[i], v[next], &fake_parent_region, 0, 0, stack);
    edge_index[i] = i;
  }
  
  return x3d_rasterregion_construct_from_edges(dest, stack, edges, edge_index, total_v);
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
  span->left =  X3D_MAX(parent_span.left, span->left);
  span->right = X3D_MIN(parent_span.right, span->right);
  
  return span->left <= span->right;
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
static _Bool x3d_rasterregion_point_inside(X3D_RasterRegion* region, X3D_Vex2D p) {
  if(p.y < region->rect.y_range.min || p.y > region->rect.y_range.max)
    return X3D_FALSE;
  
  uint16 offsety = p.y - region->rect.y_range.min;
  
  return p.x >= region->span[offsety].left && p.x <= region->span[offsety].right;
}


int16 x3d_clip_line_to_near_plane(X3D_Vex3D* a, X3D_Vex3D* b, X3D_Vex2D* a_project, X3D_Vex2D* b_project, X3D_Vex2D* a_dest, X3D_Vex2D* b_dest, int16 z) {
  if(a->z < z && b->z < z) {
    return EDGE_INVISIBLE | EDGE_NEAR_CLIPPED;
  }
  
  // Quick rejection against pseudo view frustum planes
  
 //  if(!x3d_key_down(X3D_KEY_15)) {
  // Bottom plane
  
  if(a->y > a->z && b->y > b->z) {
    //printf("INVISIBLE!!!\n");
    //return EDGE_INVISIBLE;
    return EDGE_INVISIBLE | EDGE_BOTTOM_CLIPPED;
  }
  
  // Top plane
  if(a->y < -a->z && b->y < -b->z) {
    //printf("INVISIBLE!!!\n");
    //return EDGE_INVISIBLE;
    return EDGE_INVISIBLE | EDGE_TOP_CLIPPED;
  }
  
 
  // Left
  if(a->x < -a->z && b->x < -b->z)
    return EDGE_INVISIBLE | EDGE_LEFT_CLIPPED;

  // Right
  if(a->x > a->z && b->x > b->z)
    return EDGE_INVISIBLE | EDGE_RIGHT_CLIPPED;
  
   //}
  
  if(a->z >= z && b->z >= z) {
    *a_dest = *a_project;
    *b_dest = *b_project;
    return 0;
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
  
  return 0;//EDGE_NEAR_CLIPPED;
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
  
  x3d_rasteredge_generate(&edge, *start, *end, region, 0, 0, stack);
  
  int16 y_min = X3D_MAX(region->rect.y_range.min, edge.rect.y_range.min);
  int16 y_max = X3D_MIN(region->rect.y_range.max, edge.rect.y_range.max);
  
  if(edge.flags & EDGE_INVISIBLE || y_min > y_max) {
    x3d_stack_restore(stack, stack_ptr);
    return X3D_FALSE;
  }
  
  // Horizontal lines
  if(start->y == end->y) {
    int16 y_offset = start->y - region->rect.y_range.min;
    
    if(start->x > end->x) {
      X3D_SWAP(start, end);
    }
    
    if(start->x < region->span[y_offset].left)
      start->x = region->span[y_offset].left;
    
    if(end->x > region->span[y_offset].right)
      end->x = region->span[y_offset].right;
    
    x3d_stack_restore(stack, stack_ptr);
    
    return start->x < end->x;
    
  }
  
  int16 i;
  _Bool found = X3D_FALSE;
  int16 x, left, right;

  // Find where the line enters the region
  for(i = y_min; i <= y_max; ++i) {
    left = region->span[i - region->rect.y_range.min].left;
    right = region->span[i - region->rect.y_range.min].right;
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
  
  for(i = region->rect.y_range.min; i < region->rect.y_range.max; ++i) {
    uint16 index = i - region->rect.y_range.min;
    x3d_screen_draw_line(region->span[index].left, i, region->span[index].right, i, color);
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
  start->x = edge->start.x;
  start->y = edge->start.y;
  
  end->x = edge->end.x;
  end->y = edge->end.y;
  
  #if 0
  x3d_assert(!(edge->flags & EDGE_INVISIBLE));
  
  if(edge->flags & EDGE_HORIZONTAL) {
    start->x = edge->x_range.min;
    start->y = edge->rect.y_range.min;
    
    end->x = edge->x_range.max;
    end->y = edge->rect.y_range.min;
  }
  else {
    start->x = edge->x_data[0];
    start->y = edge->rect.y_range.min;
    
    end->x = edge->x_data[edge->rect.y_range.max - edge->rect.y_range.min];
    end->y = edge->rect.y_range.max;
  }
  
  if(edge->flags & EDGE_V_SWAPPED)
    X3D_SWAP(*start, *end);
#endif
}

/// Under construction
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
  
  for(i = 0; i < clip->total_edge_index; ++i) {
    X3D_Pair edge = clip->edge_pairs[clip->edge_index[i]];
    uint16 in[2] = { clip->v3d[edge.val[0]].z >= near_z, clip->v3d[edge.val[1]].z >= near_z };

    _Bool frustum_clipped = X3D_FALSE;
    
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
    
    if((in[0] || in[1]) && !frustum_clipped) {
      vis_e[total_vis_e++] = clip->edge_index[i];
      
      if(in[0] != in[1]) {
        X3D_Vex2D v[2];
        
        x3d_rasteredge_get_endpoints(clip->edges + clip->edge_index[i], v, v + 1);
        
        // FIXME
        if(in[0]) {
          out_v[total_out_v] = v[1];
          depth[total_out_v++] = clip->edges[clip->edge_index[i]].start.z;
        }
        else {
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
  
  //x3d_assert(total_out_v <= 2);
    
  // Create a two edge between the two points clipped by the near plane
  if(total_out_v == 2) { 
    /// FIXME please!
#if 1
    x3d_rasteredge_generate(clip->edges + total_e,
      out_v[0], out_v[1], clip->parent, depth[0], depth[1], &renderman->stack);
    
    
    //printf("Line: {%d,%d} - {%d,%d}, %d\n", out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y, total_vis_e + 1);
    
    //x3d_screen_draw_line(out_v[0].x, out_v[0].y, out_v[1].x, out_v[1].y, 0x7FFF);
    
    
    vis_e[total_vis_e++] = total_e++;
#endif
  }
  
#if 0
  if(top_clipped && x3d_key_down(X3D_KEY_15)) {
    X3D_Vex2D a = { 0, 0 };
    X3D_Vex2D b = { x3d_screenmanager_get()->w - 1, 0 };
    
    x3d_rasteredge_generate(clip->edges + total_e,
      a, b, clip->parent, 10, 10, &renderman->stack);
    
    vis_e[total_vis_e++] = total_e++;
  }
  
  if(bottom_clipped && x3d_key_down(X3D_KEY_15)) {
    X3D_Vex2D a = { 0, x3d_screenmanager_get()->h - 1 };
    X3D_Vex2D b = { x3d_screenmanager_get()->w - 1, x3d_screenmanager_get()->h - 1 };
    
    x3d_rasteredge_generate(clip->edges + total_e,
      a, b, clip->parent, 10, 10, &renderman->stack);
    
    vis_e[total_vis_e++] = total_e++;
  }
#endif
  
  //printf("Total vis e: %d\nOut v: %d\n", total_vis_e, total_out_v);
  
  //return total_vis_e > 0 &&
  if(total_vis_e > 2) {
    if(x3d_rasterregion_construct_from_edges(dest, &renderman->stack, clip->edges, vis_e, total_vis_e)) {
      if(total_out_v == 2 && x3d_rasterregion_clip_line(clip->parent, &renderman->stack, out_v, out_v + 1)) {
        uint16 i;
        
        int16 y_index = out_v[0].y - dest->rect.y_range.min;
        
        //x3d_assert(y_index >= dest->y_range.min);
        
        if(y_index < dest->rect.y_range.min)
          y_index = 0;
        
        if(abs(out_v[0].x - dest->span[y_index].left) < abs(out_v[0].x - dest->span[y_index].right)) {
          //printf("Left!\n");
          
          if(!x3d_key_down(X3D_KEY_15)) {
            for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
              dest->span[i].left = 0;
          }
        }
        else {
          if(!x3d_key_down(X3D_KEY_15)) {
            for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
              dest->span[i].right = x3d_screenmanager_get()->w - 1;
          }
          
          //printf("Right!\n");
        }
      }
      
      if(left_clipped && x3d_key_down(X3D_KEY_15)) {
        for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
          dest->span[i].left = 0;
      }
      
      if(right_clipped && x3d_key_down(X3D_KEY_15)) {
        for(i = 0; i < dest->rect.y_range.max - dest->rect.y_range.min + 1; ++i)
          dest->span[i].right = x3d_screenmanager_get()->w - 1;
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

