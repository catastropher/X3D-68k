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

/// @todo Document.
typedef struct X3D_BoundSphere {
  int16 r;
  X3D_Vex3D center;
} X3D_BoundSphere;

typedef struct X3D_RayCaster {
  uint16 seg;                 // Segment the raycaster is currently in
  X3D_Vex3D_fp16x8 pos;       // Position of the raycaster
  X3D_Vex3D_fp0x16 dir;
  X3D_SegFaceID hit_face;
  
  X3D_Vex3D hit_pos;
  int16 dist;
} X3D_RayCaster;

void x3d_raycaster_init(X3D_RayCaster* caster, uint16 seg_id, X3D_Vex3D_fp16x8 pos, X3D_Vex3D_fp0x16 dir);
void x3d_raycaster_cast(X3D_RayCaster* caster);


