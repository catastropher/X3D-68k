#pragma once

#include "X3D_common.h"
#include "render/X3D_polyvertex.h"
#include "X3D_clip.h"

///////////////////////////////////////////////////////////////////////////////
/// Holds the slope information for the scanline generator including:
///   - texture coordinates divided by depth (u, v)
///   - inverse depth (z)
///   - x
///   - color intensity
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_ScanlineSlope {
  int32 u;
  int32 v;
  int32 z;
  int32 intensity;
  int32 x;
} X3D_ScanlineSlope;

struct X3D_RasterRegion;

///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_ScanlineGenerator {
  struct X3D_RasterRegion* dest;        ///< Raster region to store scanlines in
  struct X3D_RasterRegion* parent;      ///< Parent that dest is inside of
  X3D_PolyVertex* a;                    ///< Top vertex of current edge
  X3D_PolyVertex* b;                    ///< Bottom vertex of current edge 
  X3D_SpanValue* span;                  ///< Span value at the current y value
  X3D_ScanlineSlope slope;              ///< Slope of interpolants
  fp16x16 x;                            ///< Current x
  fp16x16 intensity;                    ///< Current intensity
  fp16x16 intensity_slope;              ///< Slope of intensity (move to X3D_ScanlineSlope)
  fp16x16 u, v;                         ///< Current u/v
  fp16x16 z;                            ///< Current z
  X3D_PolyVertex temp_a;                ///< Temp vertices for clipping
  X3D_PolyVertex temp_b;
  X3D_Range intersect_range;
  _Bool prev_visible_edge;
  X3D_Range y_range;
  X3D_PolyLine* line;
  X3D_PolyLine* other_side;             ///< The polyline on the other side of what we're working on
  int32 y;
} X3D_ScanlineGenerator;


void x3d_rasterregion_cheat_calc_texture(X3D_RasterRegion* region, X3D_PolyLine* p_left, X3D_PolyLine* p_right, X3D_RasterRegion* parent);
_Bool x3d_scanline_generator_set_edge(X3D_ScanlineGenerator* gen, X3D_PolyVertex* a, X3D_PolyVertex* b);
void x3d_rasterregion_generate_new_spans(X3D_ScanlineGenerator* gen, int16 start_y, int16 end_y);
void x3d_rasterregion_generate_spans_a_in_b_out(X3D_ScanlineGenerator* gen, int16 end_y);
void x3d_rasterregion_generate_spans_a_out_b_in(X3D_ScanlineGenerator* gen, int16 end_y);
void x3d_span_get_spanvalue_at_x(X3D_PolyVertex left, X3D_PolyVertex right, int16 x, X3D_SpanValue2* dest);
void x3d_rasterregion_generate_spans_a_out_b_out(X3D_ScanlineGenerator* gen, int16 end_y);
void x3d_rasterregion_copy_intersection_spans(X3D_ScanlineGenerator* gen, X3D_Vex2D* clip, int16 start_y, int16 end_y, _Bool left);
int16 x3d_rasterregion_edge_x_value(X3D_ScanlineGenerator* gen, int16 y);
void x3d_scanline_slope_calc(X3D_ScanlineSlope* slope, const X3D_PolyVertex* a, const X3D_PolyVertex* b);

