
#pragma once

#include "X3D_common.h"
#include "X3D_vector.h"
#include "memory/X3D_stack.h"
#include "X3D_screen.h"

typedef struct X3D_SpanValue {
  int16 x;
  int16 intensity;
  int16 u, v;
  int16 z;
} X3D_SpanValue;

typedef struct X3D_Span {
  X3D_SpanValue left, right;
  int16 old_left_val, old_right_val;
  fp0x16 left_scale, right_scale;
} X3D_Span;

typedef struct X3D_Range {
  int32 min;
  int32 max;
} X3D_Range;

#define SCREEN_X_RANGE get_range(0, LCD_WIDTH - 1)
#define SCREEN_Y_RANGE get_range(0, LCD_HEIGHT - 1)



static inline X3D_Range get_range(int32 a, int32 b) {
  return (X3D_Range) { X3D_MIN(a, b), X3D_MAX(a, b) };
}

static inline _Bool range_overlap(X3D_Range a, X3D_Range b) {
  return !(a.max < b.min || a.min > b.max);
}

static inline _Bool in_range(X3D_Range r, int16 val) {
  return val >= r.min && val <= r.max;
}

#include "X3D_rasteredge.h"

enum {
  EDGE_HORIZONTAL = 1,
  EDGE_INVISIBLE = 2,
  EDGE_NEAR_CLIPPED = 4,
  EDGE_LEFT_CLIPPED = 8,
  EDGE_RIGHT_CLIPPED = 16,
  EDGE_BOTTOM_CLIPPED = 32,
  EDGE_TOP_CLIPPED = 64,
  EDGE_NO_DRAW = 128,
  EDGE_SWAPPED = 256
};

typedef X3D_Stack X3D_RenderStack;

typedef struct X3D_RasterRegion {
  X3D_BoundRange rect;
  X3D_Span* span;
  int16 extreme_left_y;
  int16 extreme_right_y;
} X3D_RasterRegion;


struct X3D_ClipContext;

_Bool x3d_rasterregion_construct_from_points(X3D_Stack* stack, X3D_RasterRegion* dest, X3D_Vex2D* v, uint16 total_v);
_Bool x3d_rasterregion_clip_line(X3D_RasterRegion* region, X3D_Stack* stack, X3D_Vex2D* start, X3D_Vex2D* end);
void x3d_rasteredge_generate(X3D_RasterEdge* edge, X3D_Vex2D a, X3D_Vex2D b, X3D_RasterRegion* parent, int16 depth_a, int16 depth_b, X3D_Stack* stack, fp0x16 scale_a, fp0x16 scale_b);
_Bool x3d_rasterregion_construct_from_edges(X3D_RasterRegion* region, X3D_RasterRegion* parent, X3D_Stack* stack, X3D_RasterEdge raster_edge[], int16 edge_index[], int16 total_e);
_Bool x3d_rasterregion_intersect(X3D_RasterRegion* region, X3D_RasterRegion* portal);
int16 x3d_clip_line_to_near_plane(X3D_Vex3D* a, X3D_Vex3D* b, X3D_Vex2D* a_project, X3D_Vex2D* b_project, X3D_Vex2D* a_dest, X3D_Vex2D* b_dest, int16 z);
void x3d_rasterregion_fill(X3D_RasterRegion* region, X3D_Color color);
void x3d_rasteredge_get_endpoints(X3D_RasterEdge* edge,  X3D_Vex2D* start, X3D_Vex2D* end);
_Bool x3d_rasterregion_point_inside(X3D_RasterRegion* region, X3D_Vex2D p);

_Bool x3d_rasterregion_construct_clipped(struct X3D_ClipContext* clip, X3D_RasterRegion* dest);
void x3d_rasteredge_set_intensity(X3D_RasterEdge* edge, fp0x16 ia, fp0x16 ib);

void x3d_rasterregion_fill_zbuf(X3D_RasterRegion* region, X3D_Color color, int16 z);

void x3d_rasterregion_draw(X3D_Vex2D* v, uint16 total_v, X3D_Color c, X3D_RasterRegion* parent, int16 z, X3D_Vex3D* normal, X3D_Vex3D* v3d);


fp16x16 x3d_val_slope(int16 d_a, int16 d_b);
X3D_Span* x3d_rasterregion_get_span(X3D_RasterRegion* r, int16 y);
void x3d_rasterregion_fill_texture(X3D_RasterRegion* r, int16 z);
