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

#include "X3D_manager.h"
#include "X3D_keys.h"

struct X3D_Camera;

/// @todo document
typedef struct X3D_Context {
  uint16 frame;
  
  struct X3D_Camera* cam;
  X3D_SelectSpinner spinner;
  
  X3D_KeyState keys;
  
  X3D_SegmentManager segment_manager;
  X3D_ObjectManager object_manager;
  
  INT_HANDLER old_int_5;
  INT_HANDLER old_int_1;

  uint16 render_clock;
  
  uint16 render_step;
  
  char status_bar[64];
  
  void* screen_data;
  
  uint16* key_data;
  uint16 key_data_size;
  _Bool record;
  uint16 play_pos;
  _Bool play;
  
  uint16 play_start;
  
  _Bool quit;
} X3D_Context;

