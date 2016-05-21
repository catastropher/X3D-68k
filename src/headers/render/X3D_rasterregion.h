#pragma once

#include "X3D_clip.h"
#include "X3D_common.h"
#include "render/X3D_polyvertex.h"

void x3d_rasterregion_find_point_inside_left(X3D_RasterRegion* r, X3D_Vex2D left_in, X3D_Vex2D left_out, X3D_Vex2D* dest);
void x3d_rasterregion_find_point_inside_right(X3D_RasterRegion* r, X3D_Vex2D right_in, X3D_Vex2D right_out, X3D_Vex2D* dest);
_Bool x3d_rasterregion_point_inside2(X3D_RasterRegion* region, X3D_Vex2D p);
void x3d_rasterregion_bin_search(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region);
X3D_Span* x3d_rasterregion_get_span(X3D_RasterRegion* r, int16 y);
_Bool x3d_rasterregion_make(X3D_RasterRegion* dest, X3D_PolyVertex* v, uint16 total_v, X3D_RasterRegion* parent, _Bool texture);
void x3d_rasterregion_update(X3D_RasterRegion* r);
uint16 x3d_rasterregion_total_spans(X3D_RasterRegion* r);
void x3d_rasterregion_downgrade(X3D_RasterRegion* r);
void x3d_rasterregion_generate_polyline_spans(X3D_RasterRegion* dest, X3D_RasterRegion* parent, X3D_PolyLine* p, X3D_PolyLine* other, int16 min_y, int16 max_y, X3D_SpanValue* spans, X3D_Range* y_range);
_Bool x3d_rasterregion_point_inside2(X3D_RasterRegion* region, X3D_Vex2D p);
void x3d_rasterregion_bin_search(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region);

