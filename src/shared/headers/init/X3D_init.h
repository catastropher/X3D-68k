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

#include "X3D_engine.h"
#include "X3D_export.h"

/// Configuration settings for X3D
/// @todo document
typedef struct {
  uint16 max_segments;            ///< Maximum number of segments the segment table
                                  ///     can hold.
  uint16 seg_pool_size;           ///
  
  uint16 screen_w, screen_h;
  uint16 context_w, context_h;
  //Vex2D context_pos; 
  uint8 fov;
  uint8 flags;
  
  uint8 log_flags;
} X3D_Config;


#ifdef __X3D_SHARED__

uint16 x3d_init_core(X3D_Context* context, X3D_Config* config);
void x3d_cleanup_core(void);


#else

#define x3d_init_core _DLL_call(uint16, (X3D_Context*, X3D_Config*), EXPORT_X3D_INIT_CORE)
#define x3d_cleanup_core _DLL_call(void, (void), EXPORT_X3D_CLEANUP_CORE)

#endif

