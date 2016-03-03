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

typedef struct X3D_BoundRect {
  X3D_Vex2D start;
  X3D_Vex2D end;
} X3D_BoundRect;

typedef struct X3D_PolyLine {
  uint16 total_v;
  X3D_Vex2D* v;
} X3D_PolyLine;

typedef struct X3D_ClipRegion {
  X3D_PolyLine left;
  X3D_PolyLine right;
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
  
  
  
  
}

void draw_polygon(X3D_Vex2D* v, uint16 total_v) {
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    uint16 n = (i + 1) % total_v;
    x3d_screen_draw_line(v[i].x, v[i].y, v[n].x, v[n].y, 31);
  }
}

void bin_search2(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region) {
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





void x3d_clipregion_test() {
}









