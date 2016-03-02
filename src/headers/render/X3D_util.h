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
#include "X3D_screen.h"

struct X3D_RasterRegion;
struct X3D_CameraObject;

int16 x3d_scale_by_depth(int16 value, int16 depth, int16 min_depth, int16 max_depth);
int16 x3d_depth_scale(int16 depth, int16 min_depth, int16 max_depth);
X3D_Color x3d_color_scale_by_depth(X3D_Color color, int16 depth, int16 min_depth, int16 max_depth);
void x3d_draw_clipped_line(int16 x1, int16 y1, int16 x2, int16 y2, int16 depth1, int16 depth2, X3D_Color color, struct X3D_RasterRegion* region);
void x3d_draw_3d_line(X3D_Vex3D a, X3D_Vex3D b, struct X3D_CameraObject* cam, X3D_Color color);

