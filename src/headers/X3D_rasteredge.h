// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "X3D_common.h"

typedef struct X3D_BoundRange {
  X3D_Range x_range;
  X3D_Range y_range;
} X3D_BoundRange;

typedef struct X3D_RasterEdge {  
  X3D_Vex3D_int32 start;
  X3D_Vex3D_int32 end;
  
  fp0x16 start_scale;
  fp0x16 end_scale;

  uint16 flags;
  
  int16* x_data;
  
  X3D_BoundRange rect; 
  
} X3D_RasterEdge;

struct X3D_RasterRegion;

void x3d_rasteredge_set_horizontal_flag(X3D_RasterEdge* edge);
void x3d_rasteredge_set_invisible_flag(X3D_RasterEdge* edge, X3D_Range region_y_range);
_Bool x3d_rasteredge_invisible(X3D_RasterEdge* edge);
_Bool x3d_rasteredge_horizontal(X3D_RasterEdge* edge);
void x3d_rasteredge_set_y_range(X3D_RasterEdge* edge, X3D_Vex2D* a, X3D_Vex2D* b);
void x3d_rasteredge_set_x_range(X3D_RasterEdge* edge, X3D_Vex2D* a, X3D_Vex2D* b);
_Bool x3d_rasteredge_frustum_clipped(X3D_RasterEdge* edge);
void x3d_rasteredge_generate(X3D_RasterEdge* edge, X3D_Vex2D a, X3D_Vex2D b, struct X3D_RasterRegion* parent, int16 depth_a, int16 depth_b, X3D_Stack* stack, fp0x16 scale_a, fp0x16 scale_b);
_Bool x3d_rasteredge_clip(X3D_RasterEdge* edge, X3D_Vex2D* a, X3D_Vex2D* b, fp16x16* slope, X3D_Range parent_y_range);
void x3d_rasteredge_get_endpoints(X3D_RasterEdge* edge,  X3D_Vex2D* start, X3D_Vex2D* end);
void x3d_rasteredge_set_intensity(X3D_RasterEdge* edge, fp0x16 ia, fp0x16 ib);

