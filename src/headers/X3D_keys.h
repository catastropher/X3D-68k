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

/// Indicates that an X3D key should be mapped to nothing
#define X3D_KEY_NONE -1

///////////////////////////////////////////////////////////////////////////////
/// Represents a logical X3D key, which is mapped to a physical
///   platform-dependent key.
///////////////////////////////////////////////////////////////////////////////
typedef enum X3D_Key {
  X3D_KEY_0 = 1,
  X3D_KEY_1 = 2,
  X3D_KEY_2 = 4,
  X3D_KEY_3 = 8,
  X3D_KEY_4 = 16,
  X3D_KEY_5 = 32,
  X3D_KEY_6 = 64,
  X3D_KEY_7 = 128,
  X3D_KEY_8 = 256,
  X3D_KEY_9 = 512,
  X3D_KEY_10 = 1024,
  X3D_KEY_11 = 2048,
  X3D_KEY_12 = 4096,
  X3D_KEY_13 = 8192,
  X3D_KEY_14 = 16384,
  X3D_KEY_15 = 32768
} X3D_Key;

typedef struct X3D_KeyManager {
  void (*key_handler)(void);
} X3D_KeyManager;

// Max number of logical keys X3D can support
#define X3D_MAX_KEYS 16

///////////////////////////////////////////////////////////////////////////////
/// Maps an SDL key to an X3D key (if using the PC port).
///
/// @param sdl_key  - an SDL key e.g. SDLK_UP
/// @param x3d_key  - an X3D key e.g. X3D_KEY_0
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM void x3d_key_map_pc(uint32 key, int32 sdl_key);


///////////////////////////////////////////////////////////////////////////////
/// Reads the keyboard to see which keys are currently being pressed.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM void x3d_read_keys();


///////////////////////////////////////////////////////////////////////////////
/// Checks whether a key is currently being held down.
///
/// @param key  - the key to check
///
/// @return Whether the key is currently being held down.
/// @note Make sure the keystate has been updated with @ref x3d_read_keys().
///////////////////////////////////////////////////////////////////////////////
_Bool x3d_key_down(uint32 key);

/// @todo document
_Bool x3d_pc_key_down(int32 key);
void x3d_pc_mouse_state(_Bool* left, _Bool* right, int16* x, int16* y);

