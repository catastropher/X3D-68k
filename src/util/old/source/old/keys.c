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

#include "X3D_keys.h"

/**
* Reads the physical keys on the keypad and updates the keystate.
*
* @param    - state to update
*
* @return nothing
* @note Make sure to call this before calling x3d_keystate_down!
*/
void x3d_keystate_update(X3D_KeyState* state) {
  int16 i;

  state->state = 0;

  for(i = 15; i >= 0; --i) {
    state->state = (state->state << 1) | _keytest(state->keys[i].row, state->keys[i].col);
  }
}

/**
* Checks if a key is being held down; if it is, it waits until the key is released.
*
* @param state    - keystate
* @param key      - key to check
*
* @return nothing
* @note This calls @ref x3d_keystate_update.
*/
_Bool x3d_keystate_down_wait(X3D_KeyState* state, X3D_Key key) {
  _Bool down;
  _Bool was_down = 0;

  do {
    down = x3d_keystate_down(state, key);
    was_down |= down;
    x3d_keystate_update(state);
  } while(down);

  return was_down;
}
