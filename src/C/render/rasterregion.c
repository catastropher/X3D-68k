#include "X3D_common.h"
#include "render/X3D_rasterregion.h"
#include "render/X3D_scanline.h"
#include "X3D_clip.h"
#include "memory/X3D_stack.h"
#include "X3D_enginestate.h"
#include "render/X3D_polyvertex.h"

#define x3d_log(...) ;

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

///////////////////////////////////////////////////////////////////////////////
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

X3D_Span* x3d_rasterregion_get_span(X3D_RasterRegion* r, int16 y) {
  return r->span + y - r->rect.y_range.min;
}

_Bool x3d_polyline_split2(X3D_PolyVertex* v, uint16 total_v, X3D_PolyLine* left, X3D_PolyLine* right);

_Bool x3d_rasterregion_make(X3D_RasterRegion* dest, X3D_PolyVertex* v, uint16 total_v, X3D_RasterRegion* parent) {
  X3D_PolyLine left, right;
  left.v = alloca(1000);
  right.v = alloca(1000);
  
  x3d_rasterregion_update(parent);
  
  /// @todo Bounding rectangle test
  
  // Split the polygon into left and right polylines
  if(!x3d_polyline_split2(v, total_v, &left, &right))
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
  x3d_rasterregion_generate_polyline_spans(dest, parent, &left, &right, min_y, max_y, &dest->span[min_y - dest->rect.y_range.min].left, &y_range_left);
  x3d_log(X3D_INFO, "=================Right=================");
  x3d_rasterregion_generate_polyline_spans(dest, parent, &right, &left, min_y, max_y, &dest->span[min_y - dest->rect.y_range.min].right, &y_range_right);
  
  
  x3d_rasterregion_cheat_calc_texture(dest, &left, &right);
  
  //x3d_polyline_draw(&left, 31);
  //x3d_polyline_draw(&right, x3d_rgb_to_color(0, 255, 0));
  
  //x3d_polyline_draw(&left, x3d_rgb_to_color(0, 255, 0));
  //x3d_polyline_draw(&right, x3d_rgb_to_color(0, 0, 255));
  
  int16 new_min = min_y;//X3D_MIN(y_range_left.min, y_range_right.min);
  int16 new_max = max_y;//X3D_MAX(y_range_left.max, y_range_right.max);

  x3d_log(X3D_INFO, "Max y: %d (real %d)", new_max, left.v[left.total_v - 1]->v2d.y);
#if 0
  while(new_min <= new_max) {
    X3D_Span* span = x3d_rasterregion_get_span(dest, new_min);
    X3D_Vex2D left = { span->left.x, new_min };
    X3D_Vex2D right = { span->right.x, new_min };
    
    if(x3d_rasterregion_point_inside(parent, left) && x3d_rasterregion_point_inside(parent, right))
      break;
    
    ++new_min;
  }
  
  while(new_min <= new_max) {
    X3D_Span* span = x3d_rasterregion_get_span(dest, new_max);
    X3D_Vex2D left = { span->left.x, new_max };
    X3D_Vex2D right = { span->right.x, new_max };
    
    if(x3d_rasterregion_point_inside(parent, left) && x3d_rasterregion_point_inside(parent, right))
      break;
    
    --new_max;
  }
#endif
  
  dest->span += new_min - dest->rect.y_range.min;
  dest->rect.y_range.min = new_min;
  dest->rect.y_range.max = new_max;
  
  
  x3d_assert(new_max <= parent->rect.y_range.max);
  
  x3d_log(X3D_INFO, "Range: %d - %d", new_min, new_max);
  
  return new_min <= new_max;
  
}

void x3d_rasterregion_update(X3D_RasterRegion* r) {
  uint16 i;
  
  int16 left = 0x7FFF;
  int16 right = -0x7FFF;
  
  for(i = r->rect.y_range.min; i <= r->rect.y_range.max; ++i) {
    uint16 index = i - r->rect.y_range.min;
    r->span[index].left.x = r->span[index].old_left_val;
    r->span[index].right.x = r->span[index].old_right_val;
    
    if(r->span[i - r->rect.y_range.min].left.x < left) {
      left = r->span[i - r->rect.y_range.min].left.x;
      r->extreme_left_y = i;
    }
    else if(r->span[i - r->rect.y_range.min].right.x > right) {
      right = r->span[i - r->rect.y_range.min].right.x;
      r->extreme_right_y = i;
    }
  }  
}

uint16 x3d_rasterregion_total_spans(X3D_RasterRegion* r) {
  return r->rect.y_range.max - r->rect.y_range.min + 1;
}

void x3d_rasterregion_downgrade(X3D_RasterRegion* r) {
  uint16 i;
  for(i = 0; i < x3d_rasterregion_total_spans(r); ++i) {
    r->span[i].old_left_val = r->span[i].left.x;
    r->span[i].old_right_val = r->span[i].right.x;
    r->span[i].left_scale = r->span[i].left.intensity;
    r->span[i].right_scale = r->span[i].right.intensity;
  }
}


void x3d_rasterregion_generate_polyline_spans(X3D_RasterRegion* dest, X3D_RasterRegion* parent, X3D_PolyLine* p, X3D_PolyLine* other, int16 min_y, int16 max_y, X3D_SpanValue* spans, X3D_Range* y_range) {
  uint16 i;
  uint16 prev = 0;
  
  X3D_ScanlineGenerator gen;
  
  gen.b = p->v[0];
  gen.parent = parent;
  gen.dest = dest;
  gen.span = spans;
  gen.x = (int32)p->v[0]->v2d.x << 16;
  gen.intensity = (int32)p->v[0]->intensity << 16;
  gen.u = (int32)p->v[0]->u << 16;
  gen.v = (int32)p->v[0]->v << 16;
  gen.z = (int32)p->v[0]->z << 16;
  gen.prev_visible_edge = X3D_FALSE;
  gen.y_range.min = 0x7FFF;
  gen.y_range.max = -0x7FFF;
  gen.line = p;
  gen.other_side = other;
  
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
        x3d_rasterregion_generate_new_spans(&gen, gen.a->v2d.y, end_y);
        gen.prev_visible_edge = X3D_TRUE;
        x3d_log(X3D_INFO, "Exit Case A");
      }
      else if(a_in) {
        x3d_log(X3D_ERROR, "Case B");
        x3d_rasterregion_generate_spans_a_in_b_out(&gen, end_y);
        x3d_log(X3D_INFO, "Exit B");
      }
      else if(b_in) {
        x3d_log(X3D_ERROR, "Case C: %d,%d - %d,%d", gen.a->v2d.x, gen.a->v2d.y, gen.b->v2d.y, gen.b->v2d.y);
        x3d_rasterregion_generate_spans_a_out_b_in(&gen, end_y);
        x3d_log(X3D_INFO, "Exit C");
      }
      else {
        x3d_log(X3D_ERROR, "Case D: %d,%d - %d,%d", gen.a->v2d.x, gen.a->v2d.y, gen.b->v2d.y, gen.b->v2d.y);
        x3d_rasterregion_generate_spans_a_out_b_out(&gen, end_y);
        x3d_log(X3D_INFO, "Exit D");
      }
    }
  }
  
  *y_range = gen.y_range;
}

