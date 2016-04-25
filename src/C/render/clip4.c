#include "X3D_common.h"
#include "render/X3D_polyvertex.h"
#include "X3D_clip.h"



void x3d_polyvertex_make(X3D_PolyVertex* v, X3D_Vex2D v2d, int32 uz, int32 vz, int32 zz, int32 i) {
  v->v2d = v2d;
  v->u = uz;
  v->v = vz;
  v->z = zz;
  v->intensity = i;
}

void x3d_polyline2_add_vertex(X3D_PolyLine2* line, X3D_PolyVertex v) {
  line->v[line->total_v++] = v;
}

enum {
  A_IN = 1,
  B_IN = 2
};

enum {
  A_IN_B_IN   = A_IN | B_IN,
  A_IN_B_OUT  = A_IN,
  A_OUT_B_IN  = B_IN,
  A_OUT_B_OUT = 0
};

typedef struct X3D_ClipperEdge {
  X3D_PolyVertex* a;
  X3D_PolyVertex* b;
} X3D_ClipperEdge;

uint16 x3d_clipper_edge_statecode(X3D_ClipperEdge edge, X3D_RasterRegion* r) {
  uint16 code = 0;
  
  if(x3d_rasterregion_point_inside(r, edge.a->v2d))
    code |= A_IN;
  
  if(x3d_rasterregion_point_inside(r, edge.b->v2d))
    code |= B_IN;
  
  return code;
}

void x3d_rasterregion_bin_search(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region);

void x3d_rasterregion_find_edge_intersection(X3D_RasterRegion* region, X3D_ClipperEdge edge, X3D_Vex2D* inter, _Bool* left) {
  x3d_rasterregion_bin_search(edge.a->v2d, edge.b->v2d, inter, region);
  
  X3D_Span* span = x3d_rasterregion_get_span(region, inter->y);
  
  *left = abs(span->left.x - inter->x) < abs(span->right.x - inter->x);
}

void x3d_polyline2_copy_from_range(X3D_PolyLine2* dest, X3D_PolyRegion* from, _Bool left, int16 start_y, int16 end_y) {
  
}


_Bool x3d_polyline_clip_to_rasterregion(X3D_PolyLine* line, X3D_RasterRegion* region, X3D_PolyLine2* dest) {
  dest->total_v = 0;
  
  x3d_polyline2_add_vertex(dest, *line->v[0]);
  
  uint16 i;
  for(i = 1; i < line->total_v; ++i) {
    X3D_ClipperEdge edge = { .a = line->v[i - 1], .b = line->v[i] };
    
    switch(x3d_clipper_edge_statecode(edge, region)) {
      case A_IN_B_IN:
        x3d_polyline2_add_vertex(dest, *edge.b);
        break;
        
      case A_IN_B_OUT:
        return X3D_FALSE;
        break;
        
      case A_OUT_B_IN:
        return X3D_FALSE;
        break;
        
      case A_OUT_B_OUT:
        return X3D_FALSE;
        break;
    }
  }
  
  return X3D_TRUE;
}

typedef struct X3D_EdgeSlope {
  X3D_PolyVertex* v;
  
  x3d_fix_slope x_slope;
  x3d_fix_slope u_slope;
  x3d_fix_slope v_slope;
  x3d_fix_slope z_slope;
  
  x3d_fix_slope x;
  x3d_fix_slope uz;
  x3d_fix_slope vz;
  x3d_fix_slope zz;
} X3D_EdgeSlope;

void x3d_polyline_rasterize_next_slope(X3D_EdgeSlope* slope) {
  X3D_PolyVertex* prev_v = slope->v;
  X3D_PolyVertex* v      = ++slope->v;
  
  int32 dy = v->v2d.y - prev_v->v2d.y;
  
  x3d_fix_slope_init(&slope->x_slope, prev_v->v2d.x, v->v2d.x, dy);
  x3d_fix_slope_init(&slope->u_slope, prev_v->u, v->u, dy);
  x3d_fix_slope_init(&slope->v_slope, prev_v->v, v->v, dy);
  x3d_fix_slope_init(&slope->z_slope, prev_v->z, v->z, dy);
  
  x3d_fix_slope_same_shift(&slope->x, &slope->x_slope, prev_v->v2d.x);
  x3d_fix_slope_same_shift(&slope->uz, &slope->u_slope, prev_v->u);
  x3d_fix_slope_same_shift(&slope->vz, &slope->v_slope, prev_v->v);
  x3d_fix_slope_same_shift(&slope->zz, &slope->z_slope, prev_v->z);
}

X3D_EdgeSlope* x3d_polyline_next_switch(X3D_EdgeSlope* left, X3D_EdgeSlope* right) {
  return (left->v->v2d.y < right->v->v2d.y ? left : right);
}

__attribute__((hot)) void x3d_polyline2_rasterize_texture(X3D_PolyLine2* left, X3D_PolyLine2* right) {
  int32 y     = left->v[0].v2d.y;
  int32 end_y = left->v[left->total_v - 1].v2d.y;
  
  X3D_EdgeSlope left_slope  = { .v = left->v };
  X3D_EdgeSlope right_slope = { .v = right->v };
  
  x3d_polyline_rasterize_next_slope(&left_slope);
  x3d_polyline_rasterize_next_slope(&right_slope);
  
  do {
    X3D_EdgeSlope* next_switch = x3d_polyline_next_switch(&left_slope, &right_slope);
    int32 next_end_y = next_switch->v->v2d.y;
    
    if(next_switch->v->v2d.y == end_y)
      ++next_end_y;
    
    
    do {
      X3D_Span2 span;
      
      span.left.x = x3d_fix_slope_val(&left_slope.x);
      span.left.u = x3d_fix_slope_val(&left_slope.uz);
      span.left.v = x3d_fix_slope_val(&left_slope.vz);
      span.left.z = x3d_fix_slope_val(&left_slope.zz);
      
      span.right.x = x3d_fix_slope_val(&right_slope.x);
      span.right.u = x3d_fix_slope_val(&right_slope.uz);
      span.right.v = x3d_fix_slope_val(&right_slope.vz);
      span.right.z = x3d_fix_slope_val(&right_slope.zz);
      
      x3d_screen_draw_scanline_texture(&span, y);
      
      x3d_fix_slope_add(&left_slope.x, &left_slope.x_slope);
      x3d_fix_slope_add(&left_slope.uz, &left_slope.u_slope);
      x3d_fix_slope_add(&left_slope.vz, &left_slope.v_slope);
      x3d_fix_slope_add(&left_slope.zz, &left_slope.z_slope);
      
      x3d_fix_slope_add(&right_slope.x, &right_slope.x_slope);
      x3d_fix_slope_add(&right_slope.uz, &right_slope.u_slope);
      x3d_fix_slope_add(&right_slope.vz, &right_slope.v_slope);
      x3d_fix_slope_add(&right_slope.zz, &right_slope.z_slope);
    } while(++y < next_end_y);
    
    if(y < end_y) {
      x3d_polyline_rasterize_next_slope(next_switch);
    }
    
  } while(y < end_y);
}

_Bool x3d_polyline_try_render(X3D_PolyLine* left, X3D_PolyLine* right, X3D_RasterRegion* region) {
  X3D_PolyLine2 new_left = { .v = alloca(1000) };
  X3D_PolyLine2 new_right = { .v = alloca(1000) };
  
  if(x3d_polyline_clip_to_rasterregion(left, region, &new_left) &&
     x3d_polyline_clip_to_rasterregion(right, region, &new_right)) {

    x3d_polyline2_rasterize_texture(&new_left, &new_right);
    
    return X3D_TRUE;
    
  }
  
  return X3D_FALSE;
}


