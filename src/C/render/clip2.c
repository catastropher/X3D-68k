// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_screen.h"
#include "X3D_keys.h"
#include "X3D_clip.h"
#include "X3D_assert.h"
#include "X3D_enginestate.h"

//#define x3d_log(...) ;

typedef struct X3D_BoundRect {
  X3D_Vex2D start;
  X3D_Vex2D end;
} X3D_BoundRect;

typedef struct X3D_ClipRegion {
  //X3D_PolyLine left;
  //X3D_PolyLine right;
} X3D_ClipRegion;



enum {
  X3D_BOUNDRECT_INTERSECT = 0,
  X3D_BOUNDRECT_FAIL_LEFT = 1,
  X3D_BOUNDRECT_FAIL_RIGHT = 2,
  X3D_BOUNDRECT_FAIL_TOP = 4,
  X3D_BOUNDRECT_FAIL_BOTTOM = 8
};

/// Determines whether a point is inside a raster region.
///
/// @param region - region
/// @param p      - point
///
/// @return Whether the point is inside the region.
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_rasterregion_point_inside2(X3D_RasterRegion* region, X3D_Vex2D p) {
  if(p.y < region->rect.y_range.min || p.y > region->rect.y_range.max)
    return X3D_FALSE;
  
  uint16 offsety = p.y - region->rect.y_range.min;
  
  return p.x >= region->span[offsety].left.x && p.x <= region->span[offsety].right.x;
}

///////////////////////////////////////////////////////////////////////////////
/// Determines whether two bounding rectangle intersect. If they don't,
///   this function also determines how b compares to a (i.e. whether
///   b is too far left of a).
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_boundrect_intersect(X3D_BoundRect* a, X3D_BoundRect* b) {
  uint16 flags = 0;
  
  if(b->end.x < a->start.x)
    flags |= X3D_BOUNDRECT_FAIL_LEFT;
  else if(b->start.x > a->end.x)
    flags |= X3D_BOUNDRECT_FAIL_RIGHT;

  if(b->end.y < a->start.y)
    flags |= X3D_BOUNDRECT_FAIL_TOP;
  else if(b->start.y > a->end.y)
    flags |= X3D_BOUNDRECT_FAIL_BOTTOM;
  
  return flags;
}

void x3d_boundrect_create(X3D_BoundRect* rect, X3D_Vex2D a, X3D_Vex2D b) {
  rect->start.x = X3D_MIN(a.x, b.x);
  rect->start.y = X3D_MIN(a.y, b.y);
  
  rect->end.x = X3D_MAX(a.x, b.x);
  rect->end.y = X3D_MAX(a.y, b.y);
}

uint16 x3d_boundrect_point_inside(X3D_BoundRect* rect, X3D_Vex2D v) {
  X3D_BoundRect temp_rect = { v, v };
  return x3d_boundrect_intersect(rect, &temp_rect);
}

void x3d_clipregion_create(X3D_ClipRegion* region, X3D_Vex2D* v, uint16 total_v) {
  
#if 0
  uint16 i;
  uint16 top_before = total_v - 2;
  uint16 top_after = 0;
  uint16 top_left = total_v - 1;
  uint16 top_right = total_v - 1;
  int16 bottom_y = INT16_MIN;
  
  uint16 prev = total_v - 1;
  
  for(i = 0; i < total_v - 1; ++i) {
    if(v[i].y < v[top_left].y) {
      top_left = i;
      top_right = i;
      top_before = prev;
      top_after = i + 1;
    }
    else if(v[i].y == v[top_left].y) {
      if(v[i].x < v[top_left].x)        top_left = i;
      else if(v[i].x > v[top_right].y)  top_right = i;
      
      if(v[prev].y != v[i].y)           top_before = prev;
      if(v[i + 1].y != v[i].y)          top_after = i + 1;
    }
    
    bottom_y = X3D_MAX(bottom_y, v[i].y);
  }
  
  region->left.v[0] = v[top_left];
  region->left.total_v = 1;
  
  region->right.v[0] = v[top_right];
  region->right.total_v = 1;
  
  // Special case for when top_before == top_after
  // This means the entire polygon consists of one line consisting of at least
  // two points that is on top of a point that on the bottom
  if(top_before == top_after) {
    region->left.v[region->left.total_v++] = v[top_before];
    region->right.v[region->right.total_v++] = v[top_before];
    
    return;
  }
  
  
  
  
  printf("Top before: %d\n", top_before);
  printf("Top after: %d\n", top_after);
  printf("Top left: %d\n", top_left);
  printf("Top right: %d\n", top_right);
  
  
#endif
  
}

void draw_polygon(X3D_Vex2D* v, uint16 total_v) {
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    uint16 n = (i + 1) % total_v;
    x3d_screen_draw_line(v[i].x, v[i].y, v[n].x, v[n].y, 31);
  }
}

void x3d_rasterregion_bin_search(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region) {
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

typedef struct X3D_PolyVertex {
  X3D_Vex2D v2d;
} X3D_PolyVertex;

typedef struct X3D_PolyLine {
  uint16 total_v;
  X3D_PolyVertex** v;
} X3D_PolyLine;

typedef struct X3D_PolyRegion {
  X3D_PolyLine left;
  X3D_PolyLine right;
} X3D_PolyRegion;

int16 x3d_line_parametric_t(X3D_Vex2D* a, X3D_Vex2D* b, X3D_Vex2D* v) {
  int16 dx = v->x - a->x;
  int16 dy = v->y - a->y;
  
  // Calculate t using whichever variable has a larger difference, to increase
  // accuracy
  if(dx > dy) {
    return (((int32)v->x - a->x) << 15) / (b->x - a->x);
  }
  else {
    return (((int32)v->y - a->y) << 15) / (b->y - a->y);
  }
}

typedef struct X3D_ScanlineGenerator {
  X3D_RasterRegion* dest;
  X3D_RasterRegion* parent;
  X3D_PolyVertex* a;
  X3D_PolyVertex* b;
  X3D_SpanValue* span;
  fp16x16 x;
  fp16x16 x_slope;
  fp16x16 intensity;
  fp16x16 intensity_slope;
  X3D_PolyVertex temp_a;
  X3D_PolyVertex temp_b;
  X3D_Range intersect_range;
  _Bool prev_visible_edge;
  X3D_Range y_range;
} X3D_ScanlineGenerator;

void x3d_scanline_generator_next(X3D_ScanlineGenerator* gen) {
  //x3d_assert(gen->span->x < 640);
  gen->x += gen->x_slope;
  gen->span = (X3D_SpanValue* )((uint8 *)gen->span + sizeof(X3D_Span));
}

void x3d_rasterregion_generate_spans_left(X3D_ScanlineGenerator* gen, int16 start_y, int16 end_y) {
  X3D_SpanValue* end_span = (X3D_SpanValue *)((uint8 *)gen->dest->span + (end_y - gen->dest->rect.y_range.min) * sizeof(X3D_Span));
  
  gen->y_range.min = X3D_MIN(gen->y_range.min, start_y);
  gen->y_range.max = X3D_MAX(gen->y_range.max, end_y - 1);
  
  x3d_log(X3D_INFO, "New min: %d", gen->y_range.min);
  x3d_log(X3D_INFO, "New max: %d", gen->y_range.max);
  
  while(gen->span < end_span) {
    gen->span->x = gen->x >> 16;
    
    //x3d_log(X3D_INFO, "X: %d", gen->span->x);
    
    x3d_scanline_generator_next(gen);
  }
}

int16 x3d_polyline_find_y_edge(X3D_PolyLine* p, int16 y) {
  // This can be replaced by a binary search... but is it worth it???
  int16 i;
  for(i = 0; i < p->total_v - 1; ++i) {
  //  if(y >= p->v[i].v2d.y && y <= p->v[i + 1].v2d.y)
  //    return i;
  }
  
  x3d_assert(!"Y value not in polyline");
}

X3D_Span* x3d_rasterregion_get_span(X3D_RasterRegion* r, int16 y) {
  return r->span + y - r->rect.y_range.min;
}

void x3d_rasterregion_copy_intersection_spans(X3D_ScanlineGenerator* gen, X3D_Vex2D* clip, int16 start_y, int16 end_y) {
  uint16 i;
    
  X3D_Span* parent_span = x3d_rasterregion_get_span(gen->parent, start_y);
  X3D_SpanValue* span_val;
  
  if(abs(parent_span->left.x - clip->x) < abs(parent_span->right.x - clip->x))
    span_val = &parent_span->left;
  else
    span_val = &parent_span->right;
  
  // Copy over the x values from the parent edge, based on which side the line intersects
  // with. But, only copy it over if there was at least one visible edge before this
  if(gen->prev_visible_edge || 1) {
    for(i = start_y; i < end_y; ++i) {
      gen->span->x = span_val->x;
      x3d_scanline_generator_next(gen);
      span_val = (X3D_SpanValue* )((uint8 *)span_val + sizeof(X3D_Span));
    }
  }
  else {
    int16 advance = end_y - start_y - 1;
    span_val = (X3D_SpanValue* )((uint8 *)span_val + sizeof(X3D_Span) * advance);
    gen->span = (X3D_SpanValue* )((uint8 *)gen->span + sizeof(X3D_Span) * advance);
    
    gen->x = (int32)span_val->x << 16;
  }
}

void x3d_rasterregion_generate_spans_a_in_b_out(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_Vex2D clip;
  x3d_rasterregion_bin_search(gen->a->v2d, gen->b->v2d, &clip, gen->parent);
  
  int16 t = x3d_line_parametric_t(&gen->a->v2d, &gen->b->v2d, &clip);
  
  // Generate the part of the span that's inside the region
  x3d_rasterregion_generate_spans_left(gen, gen->a->v2d.y, clip.y);
  
  x3d_rasterregion_copy_intersection_spans(gen, &clip, clip.y, end_y);
  
  gen->prev_visible_edge = X3D_TRUE;
}

void x3d_rasterregion_generate_spans_a_out_b_in(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_Vex2D clip;
  x3d_rasterregion_bin_search(gen->b->v2d, gen->a->v2d, &clip, gen->parent);
  
  int16 t = x3d_line_parametric_t(&gen->a->v2d, &gen->b->v2d, &clip);
  
  x3d_rasterregion_copy_intersection_spans(gen, &clip, gen->a->v2d.y, clip.y);
  
  gen->x = (int32)clip.x << 16;
  
  // Generate the part of the span that's inside the region
  x3d_rasterregion_generate_spans_left(gen, clip.y, end_y);
  
  gen->prev_visible_edge = X3D_TRUE;
}

int16 x3d_rasterregion_edge_x_value(X3D_ScanlineGenerator* gen, int16 y) {
  return gen->a->v2d.x + ((gen->x_slope) * (y - gen->a->v2d.y) >> 16);
}

void x3d_rasterregion_find_point_inside_left(X3D_RasterRegion* r, X3D_Vex2D left_in, X3D_Vex2D left_out, X3D_Vex2D* dest) {
  X3D_Vex2D mid;
  
  do {
    mid.x = (left_in.x + left_out.x) >> 1;
    mid.y = (left_in.y + left_out.y) >> 1;
    
    
    X3D_Span* span = x3d_rasterregion_get_span(r, mid.y);
    
    //x3d_log(X3D_INFO, "%d %d, %d %d - %d, %d\n", in.x, in.y, out.x, out.y, mid.x, mid.y);
    
    if(abs(mid.x - span->left.x) < 2)
      break;
    
    if(mid.x < span->left.x)
      left_out = mid;
    else
      left_in = mid;
    
  } while(1);
  
  dest->x = mid.x;
  dest->y = mid.y;
  
  //x3d_assert(x3d_rasterregion_point_inside2(r, *dest));
}

void x3d_rasterregion_find_point_inside_right(X3D_RasterRegion* r, X3D_Vex2D right_in, X3D_Vex2D right_out, X3D_Vex2D* dest) {
  X3D_Vex2D mid;
  
  do {
    mid.x = (right_in.x + right_out.x) >> 1;
    mid.y = (right_in.y + right_out.y) >> 1;
    
    
    X3D_Span* span = x3d_rasterregion_get_span(r, mid.y);
    
    //x3d_log(X3D_INFO, "%d %d, %d %d - %d, %d\n", in.x, in.y, out.x, out.y, mid.x, mid.y);
    
    if(abs(mid.x - span->right.x) < 2)
      break;
    
    if(mid.x > span->right.x)
      right_out = mid;
    else
      right_in = mid;
    
  } while(1);
  
  dest->x = mid.x;
  dest->y = mid.y;
  
  //x3d_assert(x3d_rasterregion_point_inside2(r, *dest));
}

void x3d_rasterregion_generate_spans_a_out_b_out(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_Span* span_a = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
  X3D_Span* span_b = x3d_rasterregion_get_span(gen->parent, gen->b->v2d.y);
  X3D_Vex2D point_inside;
  
  _Bool out_left = gen->a->v2d.x < span_a->left.x && gen->b->v2d.x < span_b->left.x;
  _Bool out_right = gen->a->v2d.x > span_a->right.x && gen->b->v2d.x > span_b->right.x;
  int16 extreme_y = (out_left ? gen->parent->extreme_left_y : gen->parent->extreme_right_y);
  _Bool fail_exteme_point = gen->b->v2d.y < extreme_y || gen->a->v2d.y > extreme_y;

  // Case 1: both enpoints are too far left or too far right
  if(out_left || out_right) {
    // If the extreme left point on the parent region isn't in the y range of the edge,
    // then it can't intersect the parent region
    if(fail_exteme_point) {
      x3d_log(X3D_ERROR, "Case D.left -> invisible by extreme point y");
      
      x3d_rasterregion_copy_intersection_spans(gen, &gen->a->v2d, gen->a->v2d.y, end_y);
      
      return;
    }
    
    point_inside.x = x3d_rasterregion_edge_x_value(gen, extreme_y);
    point_inside.y = extreme_y;
    
    x3d_log(X3D_INFO, "V.y: %d, %d", point_inside.y, point_inside.x);
    
    if(x3d_rasterregion_point_inside2(gen->parent, point_inside)) {
      X3D_Vex2D clip_a;
      X3D_Vex2D clip_b;
      
clip_found_inside:
      x3d_rasterregion_bin_search(point_inside, gen->a->v2d, &clip_a, gen->parent);
      
      x3d_rasterregion_bin_search(point_inside, gen->b->v2d, &clip_b, gen->parent);
      
      x3d_rasterregion_copy_intersection_spans(gen, &clip_a, gen->a->v2d.y, clip_a.y);
      x3d_rasterregion_generate_spans_left(gen, clip_a.y, clip_b.y);
      x3d_rasterregion_copy_intersection_spans(gen, &clip_a, clip_b.y, end_y);
      
      
      
      
      
      
      
      x3d_log(X3D_ERROR, "Case D.left -> visible");
    }
    else {
      x3d_log(X3D_ERROR, "Case D.left -> not visible");
      
      x3d_rasterregion_copy_intersection_spans(gen, &gen->a->v2d, gen->a->v2d.y, end_y);
    }
  }
  else {
    x3d_log(X3D_ERROR, "Case D -> left and right");
    X3D_Vex2D left = gen->a->v2d;
    X3D_Vex2D right = gen->b->v2d;
    
    X3D_Vex2D clip_a, clip_b;
    
    if(left.x > right.x)
      X3D_SWAP(left, right);
    
    x3d_rasterregion_find_point_inside_left(gen->parent, right, left, &clip_a);
    x3d_rasterregion_find_point_inside_right(gen->parent, left, right, &clip_b);
    
    if(clip_a.y > clip_b.y) {
      X3D_SWAP(clip_a, clip_b);
    }
    
    x3d_rasterregion_copy_intersection_spans(gen, &clip_a, gen->a->v2d.y, clip_a.y);
    x3d_rasterregion_generate_spans_left(gen, clip_a.y, clip_b.y);
    x3d_rasterregion_copy_intersection_spans(gen, &clip_b, clip_b.y, end_y);
    
  }
}

int16 x3d_t_clip(int16 start, int16 end, uint16 scale) {
  return start + ((((int32)end - start) * scale) >> 15);
}

_Bool x3d_scanline_generator_vertically_clip_edge(X3D_ScanlineGenerator* gen) {
  if(gen->b->v2d.y < gen->dest->rect.y_range.min)
    return X3D_FALSE;
  
  if(gen->a->v2d.y > gen->dest->rect.y_range.max)
    return X3D_FALSE;
  
  if(gen->a->v2d.y < gen->dest->rect.y_range.min) {
    // Clip the edge using the temporary a vertex
    
    int16 in = gen->b->v2d.y - gen->dest->rect.y_range.min;
    int16 out = gen->dest->rect.y_range.min - gen->a->v2d.y;
    uint16 scale = ((int32)in << 15) / (in + out);
    
    gen->temp_a.v2d.x = x3d_t_clip(gen->b->v2d.x, gen->a->v2d.x, scale);
    gen->temp_a.v2d.y = gen->dest->rect.y_range.min;
    
    gen->a = &gen->temp_a;
    
    x3d_log(X3D_INFO, "New x: %d", gen->a->v2d.x);
    x3d_log(X3D_INFO, "Scale: %d", scale);
    
    gen->x = (int32)gen->a->v2d.x << 16;
  }
  
  if(gen->b->v2d.y > gen->dest->rect.y_range.max) {
    // Clip the edge using the temporary a vertex
    
    int16 in = gen->dest->rect.y_range.max - gen->a->v2d.y;
    int16 out =  gen->b->v2d.y - gen->dest->rect.y_range.max;
    uint16 scale = ((int32)in << 15) / (in + out);
    
    gen->temp_b.v2d.x = x3d_t_clip(gen->a->v2d.x, gen->b->v2d.x, scale);
    gen->temp_b.v2d.y = gen->dest->rect.y_range.max;
    
    gen->b = &gen->temp_b;
    
    x3d_log(X3D_INFO, "New x: %d", gen->b->v2d.x);
    x3d_log(X3D_INFO, "Scale: %d", scale);
  }
  
  return X3D_TRUE;
}


_Bool x3d_scanline_generator_set_edge(X3D_ScanlineGenerator* gen, X3D_PolyVertex* a, X3D_PolyVertex* b) {
  gen->a = a;
  gen->b = b;
  
  int16 dy = b->v2d.y - a->v2d.y;

  if(dy == 0) return X3D_FALSE;
  
  gen->x_slope = (((int32)b->v2d.x - a->v2d.x) << 16) / dy;
  
  return x3d_scanline_generator_vertically_clip_edge(gen);
}

//_Bool x3d_scanline_generator_set_edge_vex2d(X3D_ScanlineGenerator)


void x3d_rasterregion_generate_polyline_spans(X3D_RasterRegion* dest, X3D_RasterRegion* parent, X3D_PolyLine* p, int16 min_y, int16 max_y, X3D_SpanValue* spans, X3D_Range* y_range) {
  uint16 i;
  uint16 prev = 0;
  
  X3D_ScanlineGenerator gen;
  
  gen.b = p->v[0];
  gen.parent = parent;
  gen.dest = dest;
  gen.span = spans;
  gen.x = (int32)p->v[0]->v2d.x << 16;
  gen.prev_visible_edge = X3D_FALSE;
  gen.y_range.min = 0x7FFF;
  gen.y_range.max = -0x7FFF;
  
  _Bool done = X3D_FALSE;
  
  for(i = 1; i < p->total_v && !done; ++i) {    
    if(x3d_scanline_generator_set_edge(&gen, p->v[i - 1], p->v[i])) {
      _Bool a_in = x3d_rasterregion_point_inside2(parent, gen.a->v2d);
      _Bool b_in = x3d_rasterregion_point_inside2(gen.parent, gen.b->v2d);
      
      // Check if this is the last edge that is visible within vertical range
      // of the parent
      int16 end_y = gen.b->v2d.y;
      if(end_y >= max_y) {
        end_y = max_y + 1;
        done = X3D_TRUE;
      }
      
      if(a_in && b_in) {
        x3d_log(X3D_INFO, "Case A");
        x3d_rasterregion_generate_spans_left(&gen, gen.a->v2d.y, end_y);
        gen.prev_visible_edge = X3D_TRUE;
      }
      else if(a_in) {
        x3d_log(X3D_ERROR, "Case B");
        x3d_rasterregion_generate_spans_a_in_b_out(&gen, end_y);
      }
      else if(b_in) {
        x3d_log(X3D_ERROR, "Case C: %d,%d - %d,%d", gen.a->v2d.x, gen.a->v2d.y, gen.b->v2d.y, gen.b->v2d.y);
        x3d_rasterregion_generate_spans_a_out_b_in(&gen, end_y);
      }
      else {
        x3d_log(X3D_ERROR, "Case D: %d,%d - %d,%d", gen.a->v2d.x, gen.a->v2d.y, gen.b->v2d.y, gen.b->v2d.y);
        x3d_rasterregion_generate_spans_a_out_b_out(&gen, end_y);
      }
    }
  }
  
  *y_range = gen.y_range;
}

///////////////////////////////////////////////////////////////////////////////
/// Splits a convex polygon into two polylines, one for the left and one for
///   the right.
///
/// @param v        - vertices
/// @param total_v  - total number of vertices
/// @param left     - left polyline dest
/// @param right    - right polyline dest
///
/// @return Whether a non-degenerate polygon remains.
/// @note   Make sure the polylines have enough space to store the result!
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_polyline_split(X3D_PolyVertex* v, uint16 total_v, X3D_PolyLine* left, X3D_PolyLine* right) {
  int16 top_left = 0;
  int16 top_right = 0;
  int16 max_y = v[0].v2d.y;
  
  int16 i;
  // Find the top left point, the top right point, and the maximum y value
  for(i = 1; i < total_v; ++i) {
    if(v[i].v2d.y < v[top_left].v2d.y) {
      top_left = i;
      top_right = i;
    }
    else if(v[i].v2d.y == v[top_left].v2d.y) {
      if(v[i].v2d.x < v[top_left].v2d.x)    top_left = i;
      if(v[i].v2d.x > v[top_right].v2d.x)   top_right = i;
    }
    
    max_y = X3D_MAX(max_y, v[i].v2d.y);
  }

  uint16 next_left = (top_left + 1 < total_v ? top_left + 1 : 0);
  uint16 prev_left = (top_left != 0 ? top_left - 1 : total_v - 1);
  
  left->total_v = 0;
  right->total_v = 0;
  
  // Check if we need to switch left and right (in the following loops the left
  // polyline is assumed to move forwards and the right polyline is assumed to move
  // backwards in the array). We need to switch if:
  //
  // 1) From the top left the next point has the same y value (y must decrease) or
  // 2) From the top left the next point has an x value > the previous point (we're
  //    the left side after all!)
  if(v[top_left].v2d.y == v[next_left].v2d.y || v[next_left].v2d.x > v[prev_left].v2d.x) {
    X3D_SWAP(left, right);
    X3D_SWAP(top_left, top_right);
  }
  
  // Grab the points for the left polyline
  do {
    left->v[left->total_v] = v + top_left;
    top_left = (top_left + 1 < total_v ? top_left + 1 : 0);
  } while(left->v[left->total_v++]->v2d.y != max_y);
  
  // Grab the points for the right polyline
  do {
    right->v[right->total_v] = v + top_right;
    top_right = (top_right != 0 ? top_right - 1 : total_v - 1);
  } while(right->v[right->total_v++]->v2d.y != max_y);
  
  // Hmm... if there is only one point on either side, should we just duplicate
  // the point to allow polygons that are 1 pixel tall?
  return left->total_v > 1 && right->total_v > 1;
}


void x3d_polyline_draw(X3D_PolyLine* p, X3D_Color c) {
  uint16 i;
  for(i = 0; i < p->total_v - 1; ++i) {
    uint16 next = (i + 1 < p->total_v ? i + 1 : 0);
    
    x3d_screen_draw_line(p->v[i]->v2d.x, p->v[i]->v2d.y, p->v[next]->v2d.x, p->v[next]->v2d.y, c);
  }
}

_Bool x3d_rasterregion_make(X3D_RasterRegion* dest, X3D_PolyVertex* v, uint16 total_v, X3D_RasterRegion* parent) {
  X3D_PolyLine left, right;
  left.v = alloca(1000);
  right.v = alloca(1000);
  
  /// @todo Bounding rectangle test
  
  // Split the polygon into left and right polylines
  if(!x3d_polyline_split(v, total_v, &left, &right))
    return X3D_FALSE;
  
  int16 min_y = X3D_MAX(parent->rect.y_range.min, left.v[0]->v2d.y);
  int16 max_y = X3D_MIN(parent->rect.y_range.max, left.v[left.total_v - 1]->v2d.y);
  
  if(min_y >= max_y)
    return X3D_FALSE;
  
  dest->rect.y_range.min = min_y;
  dest->rect.y_range.max = max_y;

  dest->span = x3d_stack_alloc(&x3d_rendermanager_get()->stack, sizeof(X3D_Span) * (max_y - min_y + 1));
  
  X3D_Range y_range_left;
  X3D_Range y_range_right;
  
  x3d_log(X3D_INFO, "=================Left=================");
  x3d_rasterregion_generate_polyline_spans(dest, parent, &left, min_y, max_y, &dest->span[min_y - dest->rect.y_range.min].left, &y_range_left);
  x3d_log(X3D_INFO, "=================Right=================");
  x3d_rasterregion_generate_polyline_spans(dest, parent, &right, min_y, max_y, &dest->span[min_y - dest->rect.y_range.min].right, &y_range_right);
  
  x3d_polyline_draw(&left, x3d_rgb_to_color(0, 255, 0));
  x3d_polyline_draw(&right, x3d_rgb_to_color(0, 0, 255));
  
  int16 new_min = X3D_MIN(y_range_left.min, y_range_right.min);
  int16 new_max = X3D_MAX(y_range_left.max, y_range_right.max);
  
  dest->span += new_min - dest->rect.y_range.min;
  dest->rect.y_range.min = new_min;
  dest->rect.y_range.max = new_max;

  x3d_assert(new_max <= parent->rect.y_range.max);
  
  x3d_log(X3D_INFO, "Range: %d - %d", new_min, new_max);
  
  return new_min <= new_max;
  
}


void x3d_rasterregion_draw_outline(X3D_RasterRegion* region, X3D_Color c) {
  uint16 i;
  
  for(i = region->rect.y_range.min; i <= region->rect.y_range.max; ++i) {
    X3D_Span* span = region->span + i - region->rect.y_range.min;
    x3d_screen_draw_pix(span->left.x, i, c);
    x3d_screen_draw_pix(span->right.x, i, x3d_rgb_to_color(0, 255, 255));
  }
}

void x3d_rasterregion_update(X3D_RasterRegion* r) {
  uint16 i;
  
  for(i = r->rect.y_range.min; i <= r->rect.y_range.max; ++i) {
    uint16 index = i - r->rect.y_range.min;
    r->span[index].left.x = r->span[index].old_left_val;
    r->span[index].right.x = r->span[index].old_right_val;
  }
}

enum {
  KEY_WIREFRAME = X3D_KEY_0,
  KEY_W = X3D_KEY_1,
  KEY_S = X3D_KEY_2,
  KEY_A = X3D_KEY_3,
  KEY_D = X3D_KEY_4,
  TEST_KEY_ESCAPE = X3D_KEY_5,
  KEY_Q = X3D_KEY_6,
  KEY_E = X3D_KEY_7,
  KEY_UP = X3D_KEY_8,
  KEY_DOWN = X3D_KEY_9,
  KEY_1 = X3D_KEY_10,
  KEY_2 = X3D_KEY_11,
  KEY_9 = X3D_KEY_12,
  KEY_0 = X3D_KEY_13,
  KEY_RECORD = X3D_KEY_14
};

_Bool key_pressed(uint16 key) {
  if(x3d_key_down(key)) {
    while(x3d_key_down(key)) x3d_read_keys();
    
    return X3D_TRUE;
  }
  
  return X3D_FALSE;
}

uint16 get_polygon(X3D_PolyVertex* v) {
  int16 cx = 640 / 2;
  int16 cy = 480 / 2;
  uint16 total_v = 0;
  
  do {
    x3d_read_keys();
    x3d_screen_draw_pix(cx, cy, 0x7FFF);
    x3d_screen_flip();
    
    if(key_pressed(KEY_WIREFRAME)) {
      v[total_v].v2d.x = cx;
      v[total_v].v2d.y = cy;
      
      ++total_v;
      
      if(total_v >= 2) {
        x3d_screen_draw_line(v[total_v - 1].v2d.x, v[total_v - 1].v2d.y, v[total_v - 2].v2d.x, v[total_v - 2].v2d.y, 31);
        x3d_screen_flip();
      }
    }
    
    if(x3d_key_down(KEY_W))  --cy;
    if(x3d_key_down(KEY_S))  ++cy;
    if(x3d_key_down(KEY_A))  --cx;
    if(x3d_key_down(KEY_D))  ++cx;
    
    SDL_Delay(10);
    
    if(key_pressed(TEST_KEY_ESCAPE))
      break;
  } while(1);
  
  return total_v;
}

uint16 x3d_rasterregion_total_spans(X3D_RasterRegion* r) {
  return r->rect.y_range.max - r->rect.y_range.min + 1;
}

void x3d_rasterregion_downgrade(X3D_RasterRegion* r) {
  uint16 i;
  for(i = 0; i < x3d_rasterregion_total_spans(r); ++i) {
    r->span[i].old_left_val = r->span[i].left.x;
    r->span[i].old_right_val = r->span[i].right.x;
    r->span[i].left_scale = 0x7FFF;
    r->span[i].right_scale = 0x7FFF;
  }
}

extern int16 render_mode;

void x3d_rasterregion_draw(X3D_Vex2D* v, uint16 total_v, X3D_Color c, X3D_RasterRegion* parent, int16 z) {
  X3D_PolyVertex pv[total_v];
  
  render_mode = 1;
  
  uint16 i;
  for(i = 0; i < total_v; ++i) {
    pv[i].v2d = v[i];
  }
  
  X3D_RasterRegion r;
  x3d_rasterregion_update(parent);
  
  if(x3d_rasterregion_make(&r, pv, total_v, parent)) {
    x3d_rasterregion_downgrade(&r);
    x3d_rasterregion_fill_zbuf(&r, c, z);
  }
}

void x3d_clipregion_test() {
  X3D_RasterRegion r;
  
  uint16 total_v = 7;
 
#if 0
  X3D_Vex2D v[] = {
    { 250, 350 },
    { 300, 350 },
    { 400, 300 },
    { 350, 200 },
    { 300, 200 },
    { 250, 200 },
    { 200, 250 }
  };
#else
  X3D_Vex2D v[] = {
    { 300, 300 },
    { 400, 300 },
    { 400, 400 },
    { 300, 400 }
  };
  
  total_v = 4;
#endif
  
  uint16 d;
  for(d = 0; d < total_v / 2; ++d)
    X3D_SWAP(v[d], v[total_v - d - 1]);
  
  x3d_screen_clear(0);
  if(!x3d_rasterregion_construct_from_points(&x3d_rendermanager_get()->stack, &r, v, total_v))
    x3d_assert(0);
  
  X3D_PolyVertex pv[30];
  
  
  x3d_screen_zbuf_clear();
  x3d_rasterregion_fill(&r, 31);
  
  x3d_rasterregion_update(&r);
  
  total_v = get_polygon(pv);
  
  int16 left = 0x7FFF;
  int16 right = -0x7FFF;
  
  uint16 i;
  for(i = r.rect.y_range.min; i <= r.rect.y_range.max; ++i) {
    if(r.span[i -r.rect.y_range.min].left.x < left) {
      left = r.span[i - r.rect.y_range.min].left.x;
      r.extreme_left_y = i;
    }
    else if(r.span[i - r.rect.y_range.min].right.x > right) {
      right = r.span[i - r.rect.y_range.min].right.x;
      r.extreme_right_y = i;
    }
  }
  
  x3d_screen_clear(0);
  x3d_screen_zbuf_clear();
  x3d_rasterregion_fill(&r, 31);
  
  
  
  
  //x3d_screen_draw_line(0, min_y, 639, min_y, 0x7FFF);
  //x3d_screen_draw_line(0, max_y, 639, max_y, 0x7FFF);
  
  //x3d_screen_draw_line(left, 0, left, 479, 0x7FFF);
  
  
  
  X3D_RasterRegion r2;
  x3d_rasterregion_make(&r2, pv, total_v, &r);
  
  x3d_rasterregion_draw_outline(&r2, x3d_rgb_to_color(255, 0, 255));
  
  
  x3d_screen_flip();
  
  X3D_RasterRegion dest;
  
  
  do {
    x3d_read_keys();
    
    if(x3d_key_down(X3D_KEY_15)) {
      while(x3d_key_down(X3D_KEY_15)) x3d_read_keys();
      break;
    }
  } while(1);
}









