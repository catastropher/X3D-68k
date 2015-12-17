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

#include "X3D_fix.h"
#include "X3D_engine.h"

//=============================================================================
// Types
//=============================================================================

typedef struct {
  uint16 max_segments;
  uint16 seg_pool_size;
  uint16 screen_w, screen_h;
  uint16 context_w, context_h;
  Vex2D context_pos; 
  uint8 fov;
  uint8 flags;
} X3D_Config;

//=============================================================================
// Functions
//=============================================================================
uint16 x3d_init(X3D_Context* context, X3D_Config* config);