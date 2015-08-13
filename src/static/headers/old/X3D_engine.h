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

#include "X3D_config.h"
#include "X3D_render.h"
#include "X3D_keys.h"

typedef struct X3D_Context {
  X3D_RenderContext context;
  X3D_EngineState state;
  X3D_RenderDevice device;
  X3D_KeyState keys;
  
#ifdef __TIGCC__
  INT_HANDLER old_int_1;
  INT_HANDLER old_int_5;
#endif
  
  uint8 quit;
} X3D_Context;
