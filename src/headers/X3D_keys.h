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


//=============================================================================
// Types
//=============================================================================

typedef enum {
  XKEY_LEFT = 1,
  XKEY_RIGHT = 2,
  XKEY_UP = 4,
  XKEY_DOWN = 8,
  XKEY_FORWARD = 16,
  XKEY_BACK = 32,
  XKEY_QUIT = 64,
  XKEY_CUSTOM1 = 128,
  XKEY_CUSTOM2 = 256,
  XKEY_CUSTOM3 = 512,
  XKEY_CUSTOM4 = 1024,
  XKEY_CUSTOM5 = 2048,
  XKEY_CUSTOM6 = 4096,
  XKEY_CUSTOM7 = 8192,
  XKEY_CUSTOM8 = 16384,
  XKEY_CUSTOM9 = 32768,
} X3D_Key;

typedef enum {
  XKEY_MAP_LEFT = 0,
  XKEY_MAP_RIGHT = 1,
  XKEY_MAP_UP = 2,
  XKEY_MAP_DOWN = 3,
  XKEY_MAP_FORWARD = 4,
  XKEY_MAP_BACK = 5,
  XKEY_MAP_QUIT = 6,
  XKEY_MAP_CUSTOM1 = 7,
  XKEY_MAP_CUSTOM2 = 8,
  XKEY_MAP_CUSTOM3 = 9,
  XKEY_MAP_CUSTOM4 = 10,
  XKEY_MAP_CUSTOM5 = 11,
  XKEY_MAP_CUSTOM6 = 12,
  XKEY_MAP_CUSTOM7 = 13,
  XKEY_MAP_CUSTOM8 = 14,
  XKEY_MAP_CUSTOM9 = 15,
} X3D_KeyMap;

/// A custom key, which holds the row and column in the keypad matrix.
typedef struct X3D_CustomKey {
  uint8 row;			///< Row
  uint8 col;			///< Column
} X3D_KeyLocation;

/// A custom key mapper that maps an X3D key to a keymatrix 
typedef struct X3D_KeyMapper {
  X3D_KeyLocation keys[16];
  uint16 state;
} X3D_KeyState;


//=============================================================================
// Function Prototypes
//=============================================================================
_Bool x3d_keystate_down_wait(X3D_KeyState* state, X3D_Key key);

//=============================================================================
// Static Inline Functions
//=============================================================================

/**
* Maps an X3D logical key to a physical key on the keypad.
*
* @param state  - keystate
* @param key    - a key from @ref X3D_KeyMap
* @param row    - row in the keypad matrix of the physical key
* @param col    - column in the keypad matrix of the physical key
*
* @return nothing
* @note Typically, you'll use this with the _rowread constants e.g. RR_UP, RR_ESC, etc.
*/
static inline void x3d_keystate_map(X3D_KeyState* state, X3D_KeyMap key, uint8 row, uint8 col) {
  state->keys[key].row = row;
  state->keys[key].col = col;
}

/**
* Checks if a key is currently being held down.
*
* @param state  - key state
* @param key    - key to check
*
* @return Whether the key is being held down.
* @note Make sure the keystate has been updated with a call to @ref x3d_keystate_update first!
*/
static inline _Bool x3d_keystate_down(X3D_KeyState* state, X3D_Key key) {
  return state->state & key;
}

