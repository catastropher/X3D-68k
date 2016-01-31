// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "X3D_common.h"
#include "X3D_prism.h"
#include "X3D_object.h"
#include "X3D_camera.h"
#include "X3D_screen.h"
#include "memory/X3D_stack.h"
#include "X3D_clip.h"


typedef struct X3D_RenderManager {
  X3D_Stack stack;
  X3D_RasterRegion region;
} X3D_RenderManager;

typedef struct X3D_DisplayLine {
  X3D_Vex3D v[2];
  X3D_Color color;
} X3D_DisplayLine;

typedef struct X3D_ClipContext {
  X3D_Stack* stack;
  X3D_RasterRegion* parent;
  X3D_RasterEdge* edges;
  uint16* edge_index;
  uint16 total_e;
  uint16 total_edge_index;
  X3D_Vex3D* v3d;
  X3D_Vex2D* v2d;
  X3D_Pair* edge_pairs;
  _Bool really_close;
} X3D_ClipContext;

#define X3D_MAX_DISPLAY_LINE 100

typedef struct X3D_DisplayLineList {
  uint16 total_l;
  X3D_DisplayLine lines[X3D_MAX_DISPLAY_LINE];
} X3D_DisplayLineList;

void x3d_prism3d_render(X3D_Prism3D* prism, X3D_CameraObject* object, X3D_Color color);
void x3d_polygon3d_render_wireframe_no_clip(X3D_Polygon3D* poly, X3D_CameraObject* object, X3D_Color color);
void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region, uint16 step);
void x3d_render(X3D_CameraObject* cam);
void x3d_draw_clipped_line(int16 x1, int16 y1, int16 x2, int16 y2, int16 depth1, int16 depth2, X3D_Color color, X3D_RasterRegion* region);

void x3d_displaylinelist_add(X3D_DisplayLineList* list, X3D_Vex2D a, int16 a_depth, X3D_Vex2D b, int16 b_depth, X3D_Color color);
void x3d_displaylinelist_render(X3D_DisplayLineList* list, X3D_RasterRegion* region);


