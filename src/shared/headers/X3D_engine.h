/* This file is part of X3D.
 *
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "X3D_config.h"
#include "debug/X3D_log.h"

/// A logical screen that is rendered to.
typedef struct X3D_RenderContext {
  uint8 flags;                ///< Flags (currently unused)

  uint8 fov;                  ///< Camera field of view
  //X3D_Vex2D_int16 pos;      ///< Position on the screen where to render
  uint16 w, h;                ///< Size of the render context

  int16 scale;                ///< Scaling factor for rendering

  //X3D_Vex2D_int16 center;   ///< Logical center of the rendering context

  uint8* screen;              ///< Screen to render to (monochrome)
  uint16 screen_w;            ///< Full witdth of the screen
  uint16 screen_h;            ///< Full height of the screen

  //X3D_Stack stack;          ///< Rendering stack

  //X3D_Camera cam;             ///< Camera

  //X3D_EngineState* state;

  uint16 render_clock;
  
  uint16 frame;

} X3D_RenderContext;


typedef struct X3D_Context {
  X3D_Log log;
  X3D_Screen screen;
  
  void (*error_handler)(uint16 code, const char* msg);
  
  //X3D_RenderContext context;
  //X3D_EngineState state;
  //X3D_RenderDevice device;
  //X3D_KeyState keys;
  
#ifdef __TIGCC__
  INT_HANDLER old_int_1;
  INT_HANDLER old_int_5;
#endif
  
  uint8 quit;
} X3D_Context;

#ifdef __X3D_SHARED__

extern uint8 x3d_loaded;


X3D_Context* x3d_get_active_context();
void x3d_set_active_context(X3D_Context* context);

#else

#define x3d_loaded _DLL_glbvar(uint8, EXPORT_X3D_LOADED)


#endif

