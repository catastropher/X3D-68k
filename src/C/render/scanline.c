#include "X3D_common.h"
#include "render/X3D_polyvertex.h"
#include "render/X3D_scanline.h"
#include "X3D_keys.h"

//#define //x3d_log(...) ;

///////////////////////////////////////////////////////////////////////////////
/// Calculates the slope of the scanline parameters.
///
/// @param slope  - scanline slope struct (dest)
/// @param a      - top edge vertex
/// @param b      - bottom vertex
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_scanline_slope_calc(X3D_ScanlineSlope* slope, const X3D_PolyVertex* a, const X3D_PolyVertex* b) {
  int16 dy = b->v2d.y - a->v2d.y;
  
  slope->x = x3d_val_slope(b->v2d.x - a->v2d.x, dy);
}

///////////////////////////////////////////////////////////////////////////////
/// Advances the scanline generator to the next scanline
///
/// @param gen - scanline generator
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
static void x3d_scanline_generator_next(X3D_ScanlineGenerator* gen) {
  gen->x         += gen->slope.x;
  gen->span = (X3D_SpanValue* )((uint8 *)gen->span + sizeof(X3D_Span));
  ++gen->y;
}

///////////////////////////////////////////////////////////////////////////////
/// Generates new span values between start_y and end_y using the current
///   slopes.
///
/// @param gen      - scanline generator
/// @param start_y  - starting y (should be where the scanline generator is
///                   currently at!)
/// @param end_y    - ending y (non-inclusive)
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_rasterregion_generate_new_spans(X3D_ScanlineGenerator* gen, int16 start_y, int16 end_y) {
  /// @todo This needs to be removed once the implementation of raster region changes
  X3D_SpanValue* end_span = (X3D_SpanValue *)((uint8 *)gen->dest->span + (end_y - gen->dest->rect.y_range.min) * sizeof(X3D_Span));
  
  /// @bug This is broken! Need to implement new strategy to find the min/max value
  //gen->y_range.min = X3D_MIN(gen->y_range.min, start_y);
  //gen->y_range.max = X3D_MAX(gen->y_range.max, end_y - 1);
  
  while(gen->span < end_span) {
    gen->span->x         = x3d_fp16x6_whole(gen->x);
    
    //x3d_screen_draw_pix(gen->span->x, gen->y, x3d_rgb_to_color(0, 255, 0));
    x3d_scanline_generator_next(gen);
  }
}

int16 x3d_t_clip(int16 start, int16 end, uint16 scale);


///////////////////////////////////////////////////////////////////////////////
/// Determines whether the current edge is potentially visible by checking
/// if it falls within the y range of the parent clipping region.
///////////////////////////////////////////////////////////////////////////////
static _Bool x3d_scanlineg_edge_visible(X3D_ScanlineGenerator* gen) {
  if(gen->b->v2d.y < gen->dest->rect.y_range.min)
    return X3D_FALSE;
  
  if(gen->a->v2d.y > gen->dest->rect.y_range.max)
    return X3D_FALSE;
  
  return X3D_TRUE;
}

void x3d_scanline_generator_clip_top(X3D_ScanlineGenerator* gen) {
  gen->temp_a = *gen->a;
  
  int16 dy = gen->dest->rect.y_range.min - gen->a->v2d.y;
  
  gen->x = (int32)gen->a->v2d.x << 16;
  gen->x += gen->slope.x * dy;
  
  gen->temp_a.v2d.x = gen->x >> 16;
  gen->temp_a.v2d.y = gen->dest->rect.y_range.min;
  
  gen->a = &gen->temp_a;

}

void x3d_scanline_generator_clip_bottom(X3D_ScanlineGenerator* gen) {
  // Clip the edge using the temporary a vertex
    
  int16 in = gen->dest->rect.y_range.max - gen->a->v2d.y;
  int16 out =  gen->b->v2d.y - gen->dest->rect.y_range.max;
  
  if(in + out == 0)
    return;
  
  int16 dy = gen->dest->rect.y_range.max - gen->a->v2d.y;
  
  gen->temp_b.v2d.x = gen->a->v2d.x + ((gen->slope.x * dy) >> 16);
  gen->temp_b.v2d.y = gen->dest->rect.y_range.max;
  
  gen->b = &gen->temp_b;
  
}

_Bool x3d_scanlineg_above_top(X3D_ScanlineGenerator* gen) {
  return gen->a->v2d.y < gen->dest->rect.y_range.min;
}

_Bool x3d_scanlineg_below_bottom(X3D_ScanlineGenerator* gen) {
  return gen->b->v2d.y > gen->dest->rect.y_range.max;
}

_Bool x3d_scanline_generator_vertically_clip_edge(X3D_ScanlineGenerator* gen) {
  if(!x3d_scanlineg_edge_visible(gen))
    return X3D_FALSE;  
  
  if(x3d_scanlineg_above_top(gen))
    x3d_scanline_generator_clip_top(gen);
  
  if(x3d_scanlineg_below_bottom(gen))
    x3d_scanline_generator_clip_bottom(gen);
  
  return X3D_TRUE;
}

_Bool x3d_scanline_generator_set_edge(X3D_ScanlineGenerator* gen, X3D_PolyVertex* a, X3D_PolyVertex* b) {
  gen->a = a;
  gen->b = b;

  int16 dy = b->v2d.y - a->v2d.y;

  if(dy == 0) return X3D_FALSE;
  
  x3d_scanline_slope_calc(&gen->slope, a, b);
  
  _Bool vis = x3d_scanline_generator_vertically_clip_edge(gen);
 
  if(vis) {
    if(gen->y != gen->a->v2d.y) {
      x3d_log(X3D_INFO, "Expected: %d (was %d)", gen->a->v2d.y, gen->y);
    }
    return X3D_TRUE;
  }
  
  return X3D_FALSE;
}

_Bool x3d_scanlineg_edge_outside_left(X3D_ScanlineGenerator* gen, X3D_Span* span_a, X3D_Span* span_b) {
  return gen->a->v2d.x < span_a->left.x && gen->b->v2d.x < span_b->left.x;
}

_Bool x3d_scanlineg_edge_outside_right(X3D_ScanlineGenerator* gen, X3D_Span* span_a, X3D_Span* span_b) {
  return gen->a->v2d.x > span_a->right.x && gen->b->v2d.x > span_b->right.x;
}

int16 x3d_scanlineg_extreme_y(X3D_ScanlineGenerator* gen, _Bool outside_left) {
  return (outside_left ? gen->parent->extreme_left_y : gen->parent->extreme_right_y);
}

_Bool x3d_scanlineg_fail_extreme_range(X3D_ScanlineGenerator* gen, int16 extreme_y) {
  return gen->b->v2d.y < extreme_y || gen->a->v2d.y > extreme_y;
}

_Bool x3d_scanlineg_pass_extreme_point(X3D_ScanlineGenerator* gen, X3D_Vex2D* point_inside, int16 extreme_y) {
  point_inside->x = x3d_rasterregion_edge_x_value(gen, extreme_y);
  point_inside->y = extreme_y;
  
  return x3d_rasterregion_point_inside2(gen->parent, *point_inside);
}

void x3d_scanlineg_a_out_b_out_same_side(X3D_ScanlineGenerator* gen, int16 extreme_y, int16 end_y) {
  x3d_log(X3D_INFO, "Out same side");
  
  ////x3d_log(X3D_INFO, "Out same side!");
  // If the extreme left point on the parent region isn't in the y range of the edge,
  // then it can't intersect the parent region
  if(x3d_scanlineg_fail_extreme_range(gen, extreme_y)) {
    x3d_log(X3D_INFO, "Fail extreme y range!");
    
    X3D_Span* span = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
    _Bool left = abs(span->left.x - gen->a->v2d.x) < abs(span->right.x - gen->a->v2d.x);
    
    x3d_rasterregion_copy_intersection_spans(gen, &gen->a->v2d, gen->a->v2d.y, end_y, left);      
    return;
  }
  
  X3D_Vex2D point_inside;
  
  if(x3d_scanlineg_pass_extreme_point(gen, &point_inside, extreme_y)) {
    X3D_Vex2D clip_a;
    X3D_Vex2D clip_b;
    
    x3d_log(X3D_INFO, "Pass extreme y test");
    
    x3d_rasterregion_bin_search(point_inside, gen->a->v2d, &clip_a, gen->parent);      
    x3d_rasterregion_bin_search(point_inside, gen->b->v2d, &clip_b, gen->parent);

    X3D_Span* span_a = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
    X3D_Span* span_b = x3d_rasterregion_get_span(gen->parent, gen->b->v2d.y);
    
    x3d_screen_draw_circle(clip_a.x, clip_a.y, 5, 0x7FFF);
    x3d_screen_draw_circle(clip_b.x, clip_b.y, 5, 31) ;
    
    _Bool left_a = abs(span_a->left.x - gen->a->v2d.x) < abs(span_a->right.x - gen->a->v2d.x);
    _Bool left_b = abs(span_b->left.x - gen->b->v2d.x) < abs(span_b->right.x - gen->b->v2d.x);

    
    x3d_rasterregion_copy_intersection_spans(gen, &clip_a, gen->a->v2d.y, clip_a.y, left_a);
    x3d_rasterregion_generate_new_spans(gen, clip_a.y, clip_b.y);
    x3d_rasterregion_copy_intersection_spans(gen, &clip_a, clip_b.y, end_y, left_b);
  }
  else {
    X3D_Span* span = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
    _Bool left = abs(span->left.x - gen->a->v2d.x) < abs(span->right.x - gen->a->v2d.x);
    
    x3d_rasterregion_copy_intersection_spans(gen, &gen->a->v2d, gen->a->v2d.y, end_y, left);          
  }
}

void x3d_scanlineg_a_out_b_out_opposite_side(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_Vex2D left = gen->a->v2d;
  X3D_Vex2D right = gen->b->v2d;
  
  X3D_Vex2D clip_a, clip_b;
  
  x3d_log(X3D_INFO, "Out opposite side");
  
  if(left.x > right.x)
    X3D_SWAP(left, right);
  
  x3d_rasterregion_find_point_inside_left(gen->parent, right, left, &clip_a);
  x3d_rasterregion_find_point_inside_right(gen->parent, left, right, &clip_b);
  
  if(clip_a.y > clip_b.y) {
    X3D_SWAP(clip_a, clip_b);
  }
  
  X3D_Span* span_a = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
  X3D_Span* span_b = x3d_rasterregion_get_span(gen->parent, gen->b->v2d.y);
  
  _Bool left_a = abs(span_a->left.x - gen->a->v2d.x) < abs(span_a->right.x - gen->a->v2d.x);
  _Bool left_b = abs(span_b->left.x - gen->b->v2d.x) < abs(span_b->right.x - gen->b->v2d.x);
  
  x3d_rasterregion_copy_intersection_spans(gen, &clip_a, gen->a->v2d.y, clip_a.y, left_a);
  x3d_rasterregion_generate_new_spans(gen, clip_a.y, clip_b.y);
  x3d_rasterregion_copy_intersection_spans(gen, &clip_b, clip_b.y, end_y, left_b);    
}

void x3d_rasterregion_generate_spans_a_out_b_out(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_Span* span_a = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
  X3D_Span* span_b = x3d_rasterregion_get_span(gen->parent, gen->b->v2d.y);
 
  /// @bug This causes an infinite loop, so we return...
  if(gen->a->v2d.y == gen->b->v2d.y) return;
  
  _Bool out_left = x3d_scanlineg_edge_outside_left(gen, span_a, span_b);
  _Bool out_right = x3d_scanlineg_edge_outside_right(gen, span_a, span_b); 
  int16 extreme_y = x3d_scanlineg_extreme_y(gen, out_left);

  // Case 1: both enpoints are too far left or too far right
  if(out_left || out_right)
    x3d_scanlineg_a_out_b_out_same_side(gen, extreme_y, end_y);
  else
    x3d_scanlineg_a_out_b_out_opposite_side(gen, end_y);
}

void x3d_rasterregion_copy_intersection_spans(X3D_ScanlineGenerator* gen, X3D_Vex2D* clip, int16 start_y, int16 end_y, _Bool left) {
  uint16 i;
    
  //gen->y_range.min = X3D_MIN(gen->y_range.min, start_y);
  //gen->y_range.max = X3D_MAX(gen->y_range.max, end_y - 1);
  
  X3D_Span* parent_span = x3d_rasterregion_get_span(gen->parent, start_y);
  X3D_SpanValue* span_val;
  
  span_val = left ? &parent_span->left : &parent_span->right;
  
  ////x3d_log(X3D_INFO, "Clip->x: %d, y: %d", clip->x, start_y);
  ////x3d_log(X3D_INFO, "left: %d, right: %d", parent_span->left.x, parent_span->right.x);
  
//   if(abs(parent_span->left.x - clip->x) == abs(parent_span->right.x - clip->x)) {
//     if(gen->b->v2d.x < gen->a->v2d.x) {
//       span_val = &parent_span->left;
//       x3d_log(X3D_INFO, "Copy from left (same)");
//     }
//     else {
//       span_val = &parent_span->right;
//       x3d_log(X3D_INFO, "Copy from right (same)");
//     }
//   }
//   else if(abs(parent_span->left.x - clip->x) < abs(parent_span->right.x - clip->x)) {
//     span_val = &parent_span->left;
//     x3d_log(X3D_INFO, "Copy from left");
//   }
//   else {
//     span_val = &parent_span->right;
//     x3d_log(X3D_INFO, "Copy from right");
//   }
  
  // Copy over the x values from the parent edge, based on which side the line intersects
  // with. But, only copy it over if there was at least one visible edge before this
  for(i = start_y; i < end_y; ++i) {
    //x3d_screen_draw_pix(span_val->x, i, 31);
    gen->span->x         = span_val->x;
    
    x3d_scanline_generator_next(gen);
    span_val = (X3D_SpanValue* )((uint8 *)span_val + sizeof(X3D_Span));
  }
}

void x3d_rasterregion_generate_spans_a_in_b_out(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_Vex2D clip;
  x3d_rasterregion_bin_search(gen->a->v2d, gen->b->v2d, &clip, gen->parent);
  
  X3D_Span* span = x3d_rasterregion_get_span(gen->parent, gen->b->v2d.y);
  _Bool left;
  
  if(abs(span->left.x - gen->b->v2d.x) < abs(span->right.x - gen->b->v2d.x)) {
    left = X3D_TRUE;
  }
  else {
    left = X3D_FALSE;
  }
  //x3d_screen_draw_circle(clip.x, clip.y, 5, 0x7FFF) ;
  
  x3d_rasterregion_generate_new_spans(gen, gen->a->v2d.y, clip.y + 1);
  x3d_rasterregion_copy_intersection_spans(gen, &clip, clip.y, end_y - 1, left);
}

void x3d_rasterregion_generate_spans_a_out_b_in(X3D_ScanlineGenerator* gen, int16 end_y) {
  // Find where the edge intersects the region
  X3D_Vex2D clip;
  x3d_rasterregion_bin_search(gen->b->v2d, gen->a->v2d, &clip, gen->parent);
  
  X3D_Span* span = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
  
  _Bool left;
  
  if(abs(span->left.x - gen->a->v2d.x) < abs(span->right.x - gen->a->v2d.x)) {
    left = X3D_TRUE;
  }
  else {
    left = X3D_FALSE;
  }

  //x3d_screen_draw_circle(gen->a->v2d.x, gen->a->v2d.y, 5, 31);
  //x3d_screen_draw_circle(clip.x, clip.y, 5, 0x7FFF);
  
  X3D_PolyVertex end;
  
  x3d_polyline_get_value(gen->line, clip.y, &end);
  
  X3D_ScanlineSlope slope = gen->slope;

  gen->slope = slope;
  
  x3d_rasterregion_copy_intersection_spans(gen, &clip, gen->a->v2d.y, clip.y, left);
  x3d_rasterregion_generate_new_spans(gen, clip.y, end_y);
}

void x3d_span_get_spanvalue_at_x(X3D_PolyVertex left, X3D_PolyVertex right, int16 x, X3D_SpanValue2* dest) {
  int32 dx = right.v2d.x - left.v2d.x;
  
  if(dx == 0) dx = 1;
  
  int32 ddx = x - left.v2d.x;
  
  if(ddx == 0) ddx = 1;
  
  dest->x = x;
  
  x3d_fix_slope u_slope, v_slope, z_slope;
  x3d_fix_slope uu, vv, zz;
  
  x3d_fix_slope_init(&u_slope, left.u, right.u, dx);
  x3d_fix_slope_same_shift(&uu, &u_slope, left.u);
  
  x3d_fix_slope_init(&v_slope, left.v, right.v, dx);
  x3d_fix_slope_same_shift(&vv, &v_slope, left.v);
  
  x3d_fix_slope_init(&z_slope, left.z, right.z, dx);
  x3d_fix_slope_same_shift(&zz, &z_slope, left.z);
  
  x3d_fix_slope_add_mul(&uu, &u_slope, ddx);
  x3d_fix_slope_add_mul(&vv, &v_slope, ddx);
  x3d_fix_slope_add_mul(&zz, &z_slope, ddx);

  dest->u = x3d_fix_slope_val(&uu);
  dest->v = x3d_fix_slope_val(&vv);
  dest->z = x3d_fix_slope_val(&zz);
}


void x3d_rasterregion_cheat_calc_texture(X3D_RasterRegion* region, X3D_PolyLine* p_left, X3D_PolyLine* p_right) {
  uint16 i;
  
  for(i = region->rect.y_range.min; i <= region->rect.y_range.max; ++i) {
    X3D_PolyVertex left, right;
    x3d_polyline_get_value(p_left, i, &left);
    x3d_polyline_get_value(p_right, i, &right);
    
    X3D_Span* span = x3d_rasterregion_get_span(region, i);
    X3D_SpanValue2 new_left, new_right;
    
    x3d_span_get_spanvalue_at_x(left, right, X3D_MAX(span->left.x, left.v2d.x), &new_left);
    x3d_span_get_spanvalue_at_x(left, right, X3D_MIN(span->right.x, right.v2d.x), &new_right);
    
    X3D_Span2 s;
    
    s.left = new_left;
    s.right = new_right;
    
    x3d_screen_draw_scanline_texture(&s, i);
  }
}

