#include "X3D_common.h"
#include "render/X3D_polyvertex.h"
#include "render/X3D_scanline.h"

///////////////////////////////////////////////////////////////////////////////
/// Calculates the slope of the scanline parameters.
///
/// @param slope -  scanline slope struct (dest)
/// @param 
void x3d_scanline_slope_calc(X3D_ScanlineSlope* slope, const X3D_PolyVertex* a, const X3D_PolyVertex* b) {
  int16 dy = b->v2d.y - a->v2d.y;
  
  slope->x = x3d_val_slope(b->v2d.x - a->v2d.x, dy);
  slope->u = x3d_val_slope(b->u - a->u,         dy);
  slope->v = x3d_val_slope(b->v - a->v,         dy);
  slope->z = x3d_val_slope(b->z - a->z,         dy);
}

// Generates new span values between start_y and end_y
void x3d_rasterregion_generate_new_spans(X3D_ScanlineGenerator* gen, int16 start_y, int16 end_y) {
  X3D_SpanValue* end_span = (X3D_SpanValue *)((uint8 *)gen->dest->span + (end_y - gen->dest->rect.y_range.min) * sizeof(X3D_Span));
  
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

// Advances the scanline generator to the next scanline
void x3d_scanline_generator_next(X3D_ScanlineGenerator* gen) {
  gen->x         += gen->slope.x;
  gen->intensity += gen->intensity_slope;
  gen->u         += gen->slope.u;
  gen->v         += gen->slope.v;
  gen->z         += gen->slope.z;
  gen->span = (X3D_SpanValue* )((uint8 *)gen->span + sizeof(X3D_Span));
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
  
  int16 span_t = 32767;//((int32)in << 15) / (in + out);
  x3d_log(X3D_INFO, "Span t: %d, %d %d", span_t, in, out);
  x3d_log(X3D_INFO, "Other side u: %d", other_side.u);
  
  int16 dy = end_y - start_y;
  
  
  int16 end_intensity = x3d_t_clip(gen->a->intensity, other_side.intensity, span_t);
  int16 end_u         = x3d_t_clip(other_side.u, end->u, span_t);
  int16 end_v         = x3d_t_clip(other_side.v, end->v, span_t);
  int16 end_z         = x3d_t_clip(other_side.z, end->z, span_t);
  
  
  
  gen->intensity_slope = x3d_val_slope(end_intensity - (gen->intensity >> 16), dy);
  gen->slope.u = x3d_val_slope(end_u - (gen->u >> 16), dy);
  gen->slope.v = x3d_val_slope(end_v - (gen->v >> 16), dy);
  gen->slope.z = x3d_val_slope(end_z - (gen->z >> 16), dy);
  
  
  x3d_log(X3D_INFO, "End u slope: %d", gen->slope.u >> 16);
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
    int16 scale = ((int32)in << 15) / (in + out);
    int16 dy = gen->dest->rect.y_range.min - gen->a->v2d.y;
    
    gen->temp_a.v2d.x = x3d_t_clip(gen->b->v2d.x, gen->a->v2d.x, scale);
    gen->temp_a.v2d.y = gen->dest->rect.y_range.min;
    
    gen->a = &gen->temp_a;
    
    x3d_log(X3D_INFO, "New x: %d", gen->a->v2d.x);
    x3d_log(X3D_INFO, "Scale: %d", scale);
    x3d_log(X3D_INFO, "!!!!!Clip top!!!!!");
    
    gen->x = (int32)gen->a->v2d.x << 16;
    gen->u += gen->slope.u * dy;
    gen->v += gen->slope.v * dy;
    gen->z += gen->slope.z * dy;
    gen->intensity += gen->intensity_slope * dy;
  }
  
  if(gen->b->v2d.y > gen->dest->rect.y_range.max) {
    // Clip the edge using the temporary a vertex
    
    int16 in = gen->dest->rect.y_range.max - gen->a->v2d.y;
    int16 out =  gen->b->v2d.y - gen->dest->rect.y_range.max;
    uint16 scale = ((int32)in << 15) / (in + out);
    int16 dy = gen->dest->rect.y_range.max - gen->a->v2d.y;
    
    gen->temp_b.v2d.x = gen->a->v2d.x + ((gen->slope.x * dy) >> 16);
    gen->temp_b.v2d.y = gen->dest->rect.y_range.max;
    
    x3d_log(X3D_INFO, "Before u: %d", gen->b->u);
    x3d_log(X3D_INFO, "Before v: %d", gen->b->v);
    x3d_log(X3D_INFO, "Slope: %d", gen->slope.u);
    x3d_log(X3D_INFO, "!!!!!Clip bottom!!!!!");
    
    
    gen->temp_b.u = gen->a->u + ((gen->slope.u * dy) >> 16);
    gen->temp_b.v = gen->a->v + ((gen->slope.v * dy) >> 16);
    gen->temp_b.z = gen->a->z + ((gen->slope.z * dy) >> 16);
    
    gen->b = &gen->temp_b;
    
    int16 ddy = gen->b->v2d.y - gen->a->v2d.y;
    //gen->slope.u = x3d_val_slope(gen->b->u - gen->a->u, ddy);
    //gen->slope.v = x3d_val_slope(gen->b->v - gen->a->v, ddy);
    
    x3d_log(X3D_INFO, "New u: %d", gen->b->u);
    x3d_log(X3D_INFO, "New v: %d", gen->b->v);
  }
  
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
      x3d_rasterregion_generate_new_spans(gen, clip_a.y, clip_b.y);
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
    x3d_rasterregion_generate_new_spans(gen, clip_a.y, clip_b.y);
    x3d_rasterregion_copy_intersection_spans(gen, &clip_b, clip_b.y, end_y);
    
  }
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
      gen->span->x = span_val->x;
      gen->span->intensity = gen->intensity >> 16;
      gen->span->u = gen->u >> 16;
      gen->span->v = gen->v >> 16;
      gen->span->z = gen->z >> 16;
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

