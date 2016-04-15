#include "X3D_common.h"
#include "render/X3D_polyvertex.h"
#include "render/X3D_scanline.h"

#define x3d_log(...) ;

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
  slope->u = x3d_val_slope(b->u - a->u,         dy);
  slope->v = x3d_val_slope(b->v - a->v,         dy);
  slope->z = x3d_val_slope(b->z - a->z,         dy);
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
  gen->intensity += gen->intensity_slope;
  gen->u         += gen->slope.u;
  gen->v         += gen->slope.v;
  gen->z         += gen->slope.z;
  gen->span = (X3D_SpanValue* )((uint8 *)gen->span + sizeof(X3D_Span));
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
  gen->y_range.min = X3D_MIN(gen->y_range.min, start_y);
  gen->y_range.max = X3D_MAX(gen->y_range.max, end_y - 1);
  
  while(gen->span < end_span) {
    gen->span->x         = x3d_fp16x6_whole(gen->x);
    gen->span->intensity = x3d_fp16x6_whole(gen->intensity);
    gen->span->u         = x3d_fp16x6_whole(gen->u);
    gen->span->v         = x3d_fp16x6_whole(gen->v);
    gen->span->z         = x3d_fp16x6_whole(gen->z);
    
    x3d_scanline_generator_next(gen);
  }
}

int16 x3d_t_clip(int16 start, int16 end, uint16 scale);

void x3d_scaline_generator_adjust_slopes(X3D_ScanlineGenerator* gen, int16 start_y, int16 end_y, int16 end_x, _Bool left, int16 edge_t, X3D_PolyVertex* end) {
  X3D_Span* end_span = x3d_rasterregion_get_span(gen->parent, end_y);
  int16 in, out;
  
  X3D_PolyVertex other_side;
  x3d_polyline_get_value(gen->other_side, end_y, &other_side);
  
  
  if(left) {
    in  = abs(other_side.v2d.x - end_span->left.x);
    out = abs(end_span->left.x - end_x);
  }
  else {
    in  = abs(other_side.v2d.x - end_span->right.x);
    out = abs(end_span->right.x - end_x);
  }
  
  if(in + out == 0)
    return;
  
  int16 span_t = ((int32)in << 15) / (in + out);
  x3d_log(X3D_INFO, "Span t: %d, %d %d", span_t, in, out);
  x3d_log(X3D_INFO, "Other side u: %d", other_side.u);
  
  int16 dy = end_y - start_y;
  
  
  int16 end_intensity = x3d_t_clip(gen->a->intensity, other_side.intensity, span_t);
  int16 end_u         = x3d_t_clip(other_side.u, end->u, span_t);
  int16 end_v         = x3d_t_clip(other_side.v, end->v, span_t);
  int16 end_z         = x3d_t_clip(other_side.z, end->z, span_t);
  
  
  gen->intensity_slope = x3d_val_slope(end_intensity - x3d_fp16x6_whole(gen->intensity), dy);
  gen->slope.u         = x3d_val_slope(end_u - x3d_fp16x6_whole(gen->u),                 dy);
  gen->slope.v         = x3d_val_slope(end_v - x3d_fp16x6_whole(gen->v),                 dy);
  gen->slope.z         = x3d_val_slope(end_z - x3d_fp16x6_whole(gen->z),                 dy);  
}

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
  int16 in = gen->b->v2d.y - gen->dest->rect.y_range.min;
  int16 out = gen->dest->rect.y_range.min - gen->a->v2d.y;
  int16 scale = ((int32)in << 15) / (in + out);
  int16 dy = gen->dest->rect.y_range.min - gen->a->v2d.y;
  
  gen->temp_a.v2d.x = x3d_t_clip(gen->b->v2d.x, gen->a->v2d.x, scale);
  gen->temp_a.v2d.y = gen->dest->rect.y_range.min;
  
  gen->a = &gen->temp_a;
  
  gen->x = (int32)gen->a->v2d.x << 16;
  gen->u += gen->slope.u * dy;
  gen->v += gen->slope.v * dy;
  gen->z += gen->slope.z * dy;
  gen->intensity += gen->intensity_slope * dy;
}

void x3d_scanline_generator_clip_bottom(X3D_ScanlineGenerator* gen) {
  // Clip the edge using the temporary a vertex
    
  int16 in = gen->dest->rect.y_range.max - gen->a->v2d.y;
  int16 out =  gen->b->v2d.y - gen->dest->rect.y_range.max;
  uint16 scale = ((int32)in << 15) / (in + out);
  int16 dy = gen->dest->rect.y_range.max - gen->a->v2d.y;
  
  gen->temp_b.v2d.x = gen->a->v2d.x + ((gen->slope.x * dy) >> 16);
  gen->temp_b.v2d.y = gen->dest->rect.y_range.max;
  
  gen->temp_b.u = gen->a->u + ((gen->slope.u * dy) >> 16);
  gen->temp_b.v = gen->a->v + ((gen->slope.v * dy) >> 16);
  gen->temp_b.z = gen->a->z + ((gen->slope.z * dy) >> 16);
  
  gen->b = &gen->temp_b;
  
  int16 ddy = gen->b->v2d.y - gen->a->v2d.y;
  //gen->slope.u = x3d_val_slope(gen->b->u - gen->a->u, ddy);
  //gen->slope.v = x3d_val_slope(gen->b->v - gen->a->v, ddy);
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
  
  return x3d_scanline_generator_vertically_clip_edge(gen);
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
  // If the extreme left point on the parent region isn't in the y range of the edge,
  // then it can't intersect the parent region
  if(x3d_scanlineg_fail_extreme_range(gen, extreme_y)) {
    x3d_rasterregion_copy_intersection_spans(gen, &gen->a->v2d, gen->a->v2d.y, end_y);      
    return;
  }
  
  X3D_Vex2D point_inside;
  
  if(x3d_scanlineg_pass_extreme_point(gen, &point_inside, extreme_y)) {
    X3D_Vex2D clip_a;
    X3D_Vex2D clip_b;
    X3D_Vex2D point_inside;
    
    x3d_rasterregion_bin_search(point_inside, gen->a->v2d, &clip_a, gen->parent);      
    x3d_rasterregion_bin_search(point_inside, gen->b->v2d, &clip_b, gen->parent);
    
    x3d_rasterregion_copy_intersection_spans(gen, &clip_a, gen->a->v2d.y, clip_a.y);
    x3d_rasterregion_generate_new_spans(gen, clip_a.y, clip_b.y);
    x3d_rasterregion_copy_intersection_spans(gen, &clip_a, clip_b.y, end_y);
  }
  else {
    x3d_rasterregion_copy_intersection_spans(gen, &gen->a->v2d, gen->a->v2d.y, end_y);
  }
}

void x3d_scanlineg_a_out_b_out_opposite_side(X3D_ScanlineGenerator* gen, int16 end_y) {
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
  x3d_rasterregion_generate_new_spans(gen, clip_a.y, clip_b.y);
  x3d_rasterregion_copy_intersection_spans(gen, &clip_b, clip_b.y, end_y);    
}

void x3d_rasterregion_generate_spans_a_out_b_out(X3D_ScanlineGenerator* gen, int16 end_y) {
  X3D_Span* span_a = x3d_rasterregion_get_span(gen->parent, gen->a->v2d.y);
  X3D_Span* span_b = x3d_rasterregion_get_span(gen->parent, gen->b->v2d.y);
  
  _Bool out_left = x3d_scanlineg_edge_outside_left(gen, span_a, span_b);
  _Bool out_right = x3d_scanlineg_edge_outside_right(gen, span_a, span_b); 
  int16 extreme_y = x3d_scanlineg_extreme_y(gen, out_left);

  // Case 1: both enpoints are too far left or too far right
  if(out_left || out_right)
    x3d_scanlineg_a_out_b_out_same_side(gen, extreme_y, end_y);
  else
    x3d_scanlineg_a_out_b_out_opposite_side(gen, end_y);
}

void x3d_rasterregion_copy_intersection_spans(X3D_ScanlineGenerator* gen, X3D_Vex2D* clip, int16 start_y, int16 end_y) {
  uint16 i;
    
  gen->y_range.min = X3D_MIN(gen->y_range.min, start_y);
  gen->y_range.max = X3D_MAX(gen->y_range.max, end_y - 1);
  
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
      gen->span->x         = span_val->x;
      gen->span->intensity = x3d_fp16x6_whole(gen->intensity);
      gen->span->u         = x3d_fp16x6_whole(gen->u);
      gen->span->v         = x3d_fp16x6_whole(gen->v);
      gen->span->z         = x3d_fp16x6_whole(gen->z);
      
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
  
  X3D_Span* span = x3d_rasterregion_get_span(gen->parent, clip.y);
  int16 x;
  _Bool left;
  
  x3d_log(X3D_INFO, "BCLIP: %d, %d -> %d", clip.x, clip.y, t);
  
  if(abs(span->left.x - clip.x) < abs(span->right.x - clip.x)) {
    x = span->left.x;
    left = X3D_TRUE;
  }
  else {
    x = span->right.x;
    left = X3D_FALSE;
  }
  
  int16 dy = gen->a->v2d.y - clip.y;
  
  if(dy != 0) {
    gen->slope.x = (((int32)gen->a->v2d.x - x) << 16) / dy;   
    // Generate the part of the span that's inside the region
    x3d_rasterregion_generate_new_spans(gen, gen->a->v2d.y, clip.y);
  }
  
  x3d_scaline_generator_adjust_slopes(gen, clip.y, end_y - 1, gen->b->v2d.x, left, t, gen->b);
  
  x3d_rasterregion_copy_intersection_spans(gen, &clip, clip.y, end_y);
  
  gen->prev_visible_edge = X3D_TRUE;
}

void x3d_rasterregion_generate_spans_a_out_b_in(X3D_ScanlineGenerator* gen, int16 end_y) {
  // Find where the edge intersects the region
  X3D_Vex2D clip;
  x3d_rasterregion_bin_search(gen->b->v2d, gen->a->v2d, &clip, gen->parent);
  
  int16 t = x3d_line_parametric_t(&gen->a->v2d, &gen->b->v2d, &clip);
  X3D_Span* span = x3d_rasterregion_get_span(gen->parent, clip.y);
  int16 x;
  
  _Bool left;
  
  if(abs(span->left.x - clip.x) < abs(span->right.x - clip.x)) {
    x = span->left.x;
    left = X3D_TRUE;
  }
  else {
    x = span->right.x;
    left = X3D_FALSE;
  }
  
  X3D_PolyVertex end;
  
  x3d_polyline_get_value(gen->line, clip.y, &end);
  
  X3D_ScanlineSlope slope = gen->slope;

  x3d_log(X3D_INFO, "\nU slope before: %d", gen->slope.u);
  
  x3d_scaline_generator_adjust_slopes(gen, gen->a->v2d.y, clip.y, clip.x, left, t, &end);
  
  x3d_log(X3D_INFO, "U slope after: %d\n", gen->slope.u);
  
  gen->slope = slope;
  
  x3d_rasterregion_copy_intersection_spans(gen, &clip, gen->a->v2d.y, clip.y);
  
  gen->slope.x = (((int32)gen->b->v2d.x - x) << 16) / (gen->b->v2d.y - clip.y); 
  
  
  gen->x = (int32)x << 16;
  
  // Generate the part of the span that's inside the region
  x3d_rasterregion_generate_new_spans(gen, clip.y, end_y);
  
  gen->prev_visible_edge = X3D_TRUE;
}

void x3d_span_get_spanvalue_at_x(X3D_PolyVertex left, X3D_PolyVertex right, int16 x, X3D_SpanValue* dest) {
  int16 dx = right.v2d.x - left.v2d.x;
  
  fp16x16 i_slope = x3d_val_slope(right.intensity - left.intensity, dx);
  fp16x16 u_slope = x3d_val_slope(right.u - left.u, dx);
  fp16x16 v_slope = x3d_val_slope(right.v - left.v, dx);
  fp16x16 z_slope = x3d_val_slope(right.z - left.z, dx);
  
  int ddx = x - left.v2d.x;
  
  dest->x = x;
  dest->intensity = left.intensity + ((i_slope * ddx) >> 16);
  dest->u = left.u + ((u_slope * ddx) >> 16);
  dest->v = left.v + ((v_slope * ddx) >> 16);
  dest->z = left.z + ((z_slope * ddx) >> 16);
}


void x3d_rasterregion_cheat_calc_texture(X3D_RasterRegion* region, X3D_PolyLine* p_left, X3D_PolyLine* p_right) {
  uint16 i;
  
  for(i = region->rect.y_range.min; i <= region->rect.y_range.max; ++i) {
    X3D_PolyVertex left, right;
    x3d_polyline_get_value(p_left, i, &left);
    x3d_polyline_get_value(p_right, i, &right);
    
    X3D_Span* span = x3d_rasterregion_get_span(region, i);
    X3D_SpanValue new_left, new_right;
    
    x3d_span_get_spanvalue_at_x(left, right, span->left.x, &new_left);
    x3d_span_get_spanvalue_at_x(left, right, span->right.x, &new_right);
    
    span->left = new_left;
    span->right = new_right;
  }
}

