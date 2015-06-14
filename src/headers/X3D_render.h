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
#include "X3D_vector.h"
#include "X3D_matrix.h"

//=============================================================================
// Structures
//=============================================================================

typedef struct {
  int16 size;
  uint8* ptr;
  uint8* base;
} X3D_Stack;

typedef struct {
  X3D_Mat3x3_fp0x16 mat;
  X3D_Vex3D_angle256 angle;
} X3D_Camera;

typedef struct X3D_RenderContext {
  uint8 flags;
  uint8 fov;
  X3D_Vex2D_int16 pos;
  uint16 w, h;

  int16 scale;

  X3D_Vex2D_int16 center;

  X3D_Stack stack;    /// Render stack

  uint8* screen;
  uint16 screen_w;
  uint16 screen_h;

  X3D_Camera cam;

} X3D_RenderContext;

typedef struct X3D_RenderDevice {
  uint8* dbuf;
} X3D_RenderDevice;

typedef struct X3D_EngineState {
  uint16 frame;
  uint16 render_delta;
} X3D_EngineState;

//=============================================================================
// Function declarations
//=============================================================================

void x3d_draw_line_black(X3D_RenderContext* context, X3D_Vex2D_int16 v1, X3D_Vex2D_int16 v2);

void x3d_rendercontext_init(X3D_RenderContext* context, uint8* screen, uint16 screen_w, uint16 screen_h, uint16 context_w,
  uint16 context_h, uint16 context_x, int16 context_y, uint8 fov, uint8 flags);

void x3d_renderdevice_init(X3D_RenderDevice* d, uint16 w, uint16 h);
void x3d_renderdevice_cleanup(X3D_RenderDevice* d);
void x3d_renderdevice_flip(X3D_RenderDevice* d);
