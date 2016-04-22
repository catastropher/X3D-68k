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
#include "X3D_clip.h"
#include "X3D_rasteredge.h"
#include "X3D_enginestate.h"


#define EDGE_VALUE(_edge, _y) ((_edge)->x_data[_y - (_edge)->rect.y_range.min])
#define EDGE(_edge) raster_edge[edge_index[_edge]]
#define REGION_OFFSET(_region, _y) (_y - _region->min_y)

#define LCD_WIDTH x3d_screenmanager_get()->w
#define LCD_HEIGHT x3d_screenmanager_get()->h

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

_Bool x3d_rasteredge_frustum_clipped(X3D_RasterEdge* edge) {
  return edge->flags & (EDGE_NEAR_CLIPPED | EDGE_LEFT_CLIPPED | EDGE_RIGHT_CLIPPED | EDGE_TOP_CLIPPED | EDGE_BOTTOM_CLIPPED);
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
      printf("Enter loop\n");
      while(a->y < -5000) {
        a->x = (a->x + b->x) / 2;
        a->y = (a->y + b->y) / 2;
      }
      
      printf("Exit loop\n");
      
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
void x3d_rasteredge_generate(X3D_RasterEdge* edge, X3D_Vex2D a, X3D_Vex2D b, X3D_RasterRegion* parent, int16 depth_a, int16 depth_b, X3D_Stack* stack, fp0x16 scale_a, fp0x16 scale_b) {
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
  if(a.y > b.y) {
    X3D_SWAP(a, b);
    X3D_SWAP(scale_a, scale_b);
    edge->flags |= EDGE_SWAPPED;
  }
  
  if(a.y < parent->rect.y_range.min && b.y > parent->rect.y_range.min) {
    int16 dist_in = abs(b.y - parent->rect.y_range.min);
    int16 dist_out = abs(a.y - parent->rect.y_range.min);
    
    int16 scale = ((int32)dist_in << 15) / (dist_in + dist_out);
    
    scale_a = scale_b + (((int32)(scale_a - scale_b) * scale) >> 15);
  }
  
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
    edge->x_data = x3d_stack_alloc(stack, height * sizeof(int16));

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
  
#if 0
  if(x3d_rasteredge_horizontal(edge) && x3d_rasteredge_invisible(edge)) {
    edge->rect.y_range.min = parent->rect.y_range.min;
    edge->rect.y_range.max = parent->rect.y_range.min;
    edge->rect.x_range.min = INT16_MAX;
    edge->rect.x_range.max = INT16_MIN;
    edge->flags &= (~EDGE_INVISIBLE);
  }
#endif
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

void x3d_rasteredge_set_intensity(X3D_RasterEdge* edge, fp0x16 ia, fp0x16 ib) {
  if(edge->flags & EDGE_SWAPPED) {
  }
  else {
  }
}


