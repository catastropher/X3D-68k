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
  X3D_PolyVertex* v;
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
  X3D_PolyVertex* a;
  X3D_PolyVertex* b;
  int16 y;
  fp16x16 x;
  fp16x16 x_slope;
  fp16x16 intensity;
  fp16x16 intensity_slope;
} X3D_ScanlineGenerator;

void x3d_rasterregion_generate_spans_left(X3D_RasterRegion* dest, X3D_ScanlineGenerator* gen, int16 end_y) {
  
}

int16 x3d_polyline_find_y_edge(X3D_PolyLine* p, int16 y) {
  // This can be replaced by a binary search... but is it worth it???
  int16 i;
  for(i = 0; i < p->total_v - 1; ++i) {
    if(y >= p->v[i].v2d.y && y <= p->v[i + 1].v2d.y)
      return i;
  }
  
  x3d_assert(!"Y value not in polyline");
}

void x3d_rasterregion_generate_spans_a_in_b_out(X3D_RasterRegion* parent, X3D_RasterRegion* dest, X3D_PolyVertex* a, X3D_PolyVertex* b) {
  X3D_Vex2D clip;
  x3d_rasterregion_bin_search(a->v2d, b->v2d, &clip, parent);
  
  int16 t = x3d_line_parametric_t(&a->v2d, &b->v2d, &clip);
  
  X3D_ScanlineGenerator gen;
  
  // Generate the part of the span that's inside the region
  x3d_rasterregion_generate_spans_left(dest, &gen, clip.y);
  
  
  
}


_Bool x3d_polyline_split(X3D_Vex2D* v, uint16 total_v, uint16 left[], uint16 right[]) {
  int16 top_left = 0;
  int16 top_right = 0;
  int16 max_y = v[0].y;
  
  int16 i;
  for(i = 1; i < total_v; ++i) {
    if (v[i].y < v[top_left].y) {
      top_left = i;
      top_right = i;
    }
    else if(v[i].y == v[top_left].y) {
      if(v[i].x < v[top_left].x)    top_left = i;
      if(v[i].x > v[top_right].x)   top_right = i;
    }
    
    max_y = X3D_MAX(max_y, v[i].y);
  }
  
  uint16 next_left = (top_left + 1 < total_v ? top_left + 1 : 0);
  
  if(v[next_left].y > v[top_left].y) {
    do {
      
    } while(1);
  }
}




void x3d_clipregion_test() {
  X3D_RasterRegion r;
  
  X3D_Vex2D v[] = {
    { 200, 200 },
    { 400, 200 },
    { 400, 400 },
    { 200, 400 }
  };
  
  x3d_screen_clear(0);
  if(!x3d_rasterregion_construct_from_points(&x3d_rendermanager_get()->stack, &r, v, 4))
    x3d_assert(0);
    
  x3d_screen_zbuf_clear();
  x3d_rasterregion_fill(&r, 31);
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









