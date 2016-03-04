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
  X3D_PolyVertex temp;
} X3D_ScanlineGenerator;

void x3d_scanline_generator_next(X3D_ScanlineGenerator* gen) {
  gen->x += gen->x_slope;
  gen->span = (X3D_SpanValue* )((uint8 *)gen->span + sizeof(X3D_Span));
}

void x3d_rasterregion_generate_spans_left(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_SpanValue* end_span = (X3D_SpanValue *)((uint8 *)gen->dest->span + (end_y - gen->dest->rect.y_range.min) * sizeof(X3D_Span));
  
  while(gen->span < end_span) {
    gen->span->x = gen->x >> 16;
    
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

void x3d_rasterregion_generate_spans_a_in_b_out(X3D_ScanlineGenerator* gen) {
  X3D_Vex2D clip;
  x3d_rasterregion_bin_search(gen->a->v2d, gen->b->v2d, &clip, gen->parent);
  
  int16 t = x3d_line_parametric_t(&gen->a->v2d, &gen->b->v2d, &clip);
  
  // Generate the part of the span that's inside the region
  x3d_rasterregion_generate_spans_left(gen, clip.y);
}

int16 x3d_t_clip(int16 start, int16 end, uint16 scale) {
  return start + ((((int32)end - start) * scale) >> 15);
}

_Bool x3d_scanline_generator_vertically_clip_edge(X3D_ScanlineGenerator* gen) {
  if(gen->b->v2d.y < gen->dest->rect.y_range.min)
    return X3D_FALSE;
  
  if(gen->a->v2d.y < gen->dest->rect.y_range.min) {
    // Clip the edge using the temporary vertex
    
    int16 in = gen->b->v2d.y - gen->dest->rect.y_range.min;
    int16 out = gen->dest->rect.y_range.min - gen->a->v2d.y;
    uint16 scale = ((int32)in << 15) / (in + out);
    
    gen->temp.v2d.x = x3d_t_clip(gen->b->v2d.x, gen->a->v2d.x, scale);
    gen->temp.v2d.y = gen->dest->rect.y_range.min;
    
    gen->a = &gen->temp;
    
    x3d_log(X3D_INFO, "New x: %d", gen->a->v2d.x);
    x3d_log(X3D_INFO, "Scale: %d", scale);
    
    gen->x = (int32)gen->a->v2d.x << 16;
  }
  
  return X3D_TRUE;
}


_Bool x3d_scanline_generator_set_edge(X3D_ScanlineGenerator* gen, X3D_PolyVertex* a, X3D_PolyVertex* b) {
  gen->a = a;
  gen->b = b;
  
  int16 dy = b->v2d.y - a->v2d.y + 1;
  
  gen->x_slope = (((int32)b->v2d.x - a->v2d.x) << 16) / dy;
  
  return x3d_scanline_generator_vertically_clip_edge(gen);
}

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


void x3d_rasterregion_generate_left(X3D_RasterRegion* dest, X3D_RasterRegion* parent, X3D_PolyLine* p, int16 min_y, int16 max_y) {
  uint16 i;
  uint16 prev = 0;
  
  X3D_ScanlineGenerator gen;
  
  gen.b = p->v[0];
  gen.parent = parent;
  gen.dest = dest;
  gen.span = &dest->span[min_y - dest->rect.y_range.min].left;
  gen.x = (int32)p->v[0]->v2d.x << 16;
  
  _Bool done = X3D_FALSE;
  
  for(i = 1; i < p->total_v && !done; ++i) {    
    if(x3d_scanline_generator_set_edge(&gen, gen.b, p->v[i])) {
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
        x3d_rasterregion_generate_spans_left(&gen, end_y);
        x3d_log(X3D_INFO, "Case A");
      }
      else if(a_in) {
        x3d_rasterregion_generate_spans_a_in_b_out(&gen);
        x3d_log(X3D_ERROR, "Case B");
      }
      else if(b_in) {
        x3d_log(X3D_ERROR, "Case C");
      }
      else {
        x3d_log(X3D_ERROR, "Case D");
      }
    }
  }
  
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
  
  // Find the top left point, the top right point, and the maximum y value
  int16 i;
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
  
  x3d_rasterregion_generate_left(dest, parent, &left, min_y, max_y);
  
  x3d_polyline_draw(&left, x3d_rgb_to_color(0, 255, 0));
  x3d_polyline_draw(&right, x3d_rgb_to_color(0, 0, 255));
  
  return X3D_TRUE;
  
}


void x3d_rasterregion_draw_outline(X3D_RasterRegion* region, X3D_Color c) {
  uint16 i;
  
  for(i = region->rect.y_range.min; i <= region->rect.y_range.max; ++i) {
    X3D_Span* span = region->span + i - region->rect.y_range.min;
    x3d_screen_draw_pix(span->left.x, i, c);
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


void x3d_clipregion_test() {
  X3D_RasterRegion r;
  
  uint16 total_v = 7;
  
  X3D_Vex2D v[] = {
    { 250, 350 },
    { 300, 350 },
    { 400, 300 },
    { 350, 200 },
    { 300, 200 },
    { 250, 200 },
    { 200, 250 }
  };
  
  uint16 d;
  for(d = 0; d < total_v / 2; ++d)
    X3D_SWAP(v[d], v[total_v - d - 1]);
  
  x3d_screen_clear(0);
  if(!x3d_rasterregion_construct_from_points(&x3d_rendermanager_get()->stack, &r, v, total_v))
    x3d_assert(0);
  
  X3D_PolyVertex pv[total_v];
  
  uint16 i;
  for(i = 0; i < total_v; ++i)
    pv[i].v2d = v[i];
  
  x3d_screen_zbuf_clear();
  x3d_rasterregion_fill(&r, 31);
  
  X3D_RasterRegion* rm = &x3d_rendermanager_get()->region;
  
  x3d_rasterregion_update(&r);
  x3d_rasterregion_update(rm);
  
  uint16 left = 150;
  
  uint16 min_y = 225;
  uint16 max_y = 300;
  
  for(i = 0; i < 480; ++i) {
    rm->span[i].old_left_val = left;
  }
  
  rm->rect.y_range.min = min_y;
  rm->rect.y_range.max = max_y;
  
  x3d_screen_draw_line(0, min_y, 639, min_y, 0x7FFF);
  x3d_screen_draw_line(0, max_y, 639, max_y, 0x7FFF);
  
  x3d_screen_draw_line(left, 0, left, 479, 0x7FFF);
  
  X3D_RasterRegion r2;
  x3d_rasterregion_make(&r2, pv, total_v, &x3d_rendermanager_get()->region);
  
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









