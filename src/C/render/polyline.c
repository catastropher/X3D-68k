#include "X3D_common.h"
#include "X3D_assert.h"
#include "render/X3D_polyvertex.h"
#include "render/X3D_scanline.h"

void print_vex2d(int16 num, X3D_Vex2D v) {
  x3d_log(X3D_INFO, "p %d: %d %d", num, v.x, v.y);
}

///////////////////////////////////////////////////////////////////////////////
/// Determines whether a polygon is clockwise given three points.
///
/// @param a - first vertex
/// @param b - second vertex
/// @param c - third vertex
///
/// @return < 0 if clockwise, > 0 if counter-clockwise, 0 if degenerate
/// @note   If all 3 points lie in a straight line this will return false.
///////////////////////////////////////////////////////////////////////////////
int32 x3d_points_clockwise(X3D_PolyVertex* a, X3D_PolyVertex* b, X3D_PolyVertex* c) {
  //int32 t1 = ((int32)b->v2d.x - a->v2d.x) * (c->v2d.y - a->v2d.y);
  //int32 t2 = ((int32)c->v2d.x - a->v2d.x) * (b->v2d.y - a->v2d.y);
  
  print_vex2d(0, a->v2d);
  print_vex2d(0, b->v2d);
  print_vex2d(0, c->v2d);
  
  x3d_log(X3D_INFO, "");
  
  
  X3D_Vex3D v1 = { b->v2d.x - a->v2d.x, b->v2d.y - a->v2d.y, 0 };
  X3D_Vex3D v2 = { c->v2d.x - b->v2d.x, c->v2d.y - b->v2d.y, 0 };
  
  X3D_Vex3D dest;
  x3d_vex3d_fp0x16_cross(&dest, &v1, &v2);
  
  return dest.z;
  
  //return (int32)v1.x * v2.y - (int32)v1.y * v2.x;
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
  uint16 next_right = (top_right != 0 ? top_right - 1 : total_v - 1);
  
  left->total_v = 0;
  right->total_v = 0;
  
  // Check if we need to switch left and right (in the following loops the left
  // polyline is assumed to move forwards and the right polyline is assumed to move
  // backwards in the array). We need to switch if:
  //
  // 1) From the top left the next point has the same y value (y must decrease) or
  // 2) From the top left the next point has an x value > the previous point (we're
  //    the left side after all!)
  
  int16 sl = X3D_SIGNOF(v[next_left].v2d.x - v[top_left].v2d.x);
  int16 sr = X3D_SIGNOF(v[next_right].v2d.x - v[top_right].v2d.x);
  
  _Bool swap = X3D_FALSE;
  
  if(v[top_left].v2d.y == v[next_left].v2d.y) {
    swap = X3D_TRUE;
    x3d_log(X3D_INFO, "PL A");
  }
  if(sl != sr || v[next_left].v2d.y == v[next_right].v2d.y) {
    swap = v[next_left].v2d.x > v[next_right].v2d.x;
    x3d_log(X3D_INFO, "PL B");
  }
  else {
    uint16 next_next_left = (next_left + 1 < total_v ? next_left + 1 : 0);
    
    swap = !x3d_points_clockwise(v + top_left, v + next_left, v + next_next_left);
  }
  
  if(swap) {
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

// Gets the span value from a polyline at a specific y value. This is expensive,
// however, and should only be done when necessary.
void x3d_polyline_get_value(X3D_PolyLine* p, int16 y, X3D_PolyVertex* v) {
  // This can be replaced by a binary search... but is it worth it???
  int16 i;
  for(i = 0; i < p->total_v - 1; ++i) {
    if(y >= p->v[i]->v2d.y && y <= p->v[i + 1]->v2d.y) {
      break;
    }
  }
  
  int16 s_dy = p->v[i + 1]->v2d.y - p->v[i]->v2d.y;
  
  X3D_PolyVertex* a = p->v[i];
  X3D_PolyVertex* b = p->v[i + 1];
  X3D_ScanlineSlope slope;
  
  x3d_scanline_slope_calc(&slope, a, b);
  
  int16 dy = y - a->v2d.y;
  
  
  v->v2d.x = a->v2d.x + ((slope.x * dy) >> 16);
  v->v2d.y = y;
  v->intensity = a->intensity + ((slope.intensity * dy) >> 16);
  v->u = a->u + ((slope.u * dy) >> 16);
  v->v = a->v + ((slope.v * dy) >> 16);
  v->z = a->z + ((slope.z * dy) >> 16);
  
}

void x3d_polyvertex_make_clockwise(X3D_PolyVertex* v, uint16 total_v) {
  int16 clockwise = 0;
  
  // Find three non-colinear points
  uint16 i;
  for(i = 0; i < total_v && clockwise == 0; ++i) {
    uint16 next = (i + 1 < total_v ? i + 1 : 0);
    uint16 next_next = (next + 1 < total_v ? next + 1 : 0);
    
    clockwise = x3d_points_clockwise(v + i, v + next, v + next_next);
  }
  
  x3d_assert(clockwise != 0);

  // Reverse the points if not clockwise
  if(clockwise > 0) {
    x3d_log(X3D_INFO, "not clockwise");
    for(i = 0; i < total_v / 2; ++i)
      X3D_SWAP(v[i], v[total_v - i - 1]);
  }
  else {
    x3d_log(X3D_INFO, "clockwise");
  }
}

_Bool x3d_polyline_split2(X3D_PolyVertex* v, uint16 total_v, X3D_PolyLine* left, X3D_PolyLine* right) {
  x3d_polyvertex_make_clockwise(v, total_v);
  
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
  
  left->total_v = 0;
  right->total_v = 0;
  
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
  
  return left->total_v > 1 && right->total_v > 1;
}


