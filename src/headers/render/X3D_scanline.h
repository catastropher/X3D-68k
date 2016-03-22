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
} X3D_ScanlineGenerator;

