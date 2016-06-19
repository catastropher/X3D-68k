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
#include "X3D_vector.h"
#include "X3D_plane.h"

struct X3D_Polygon3D;
struct X3D_CameraObject;

void x3d_raytrace_find_segface(struct X3D_CameraObject* cam, X3D_Vex2D pos, X3D_Vex3D* hit_pos, int16* hit_seg, int16* hit_face, int16* scale);
