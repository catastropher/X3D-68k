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

#include <tigcclib.h>

#include "X3D_common.h"
#include "X3D_init.h"
#include "X3D_keys.h"
#include "X3D_assert.h"


// The max X3D key that has been set (will be used to determine how many bytes)
//    of the keystate need to be transferred with multiplayer.
static int16 max_key;

// Holds which X3D keys are currently being pressed
static uint32 key_state;

// Holds which keys have been pressed since the last time checked
static uint32 key_pressed_state;

// Maps a 68k key to an X3D key
static uint16 key_map[X3D_MAX_KEYS][2];


X3D_INTERNAL void x3d_platform_keys_init(X3D_InitSettings* settings) {
  x3d_log(X3D_INFO, "Key mapper init\n");
  
  key_state = 0;
  key_pressed_state = 0;
  
  uint16 i;
  for(i = 0; i < X3D_MAX_KEYS; ++i) {
    key_map[i][0] = 0xFFFF;
  }
}

void x3d_key_map_68k(X3D_Key x3d_key, uint16 row, uint16 col) {
  int id = 0;
  
  x3d_assert(x3d_key != 0);
  
  while((x3d_key & 1) == 0) {
    x3d_key >>= 1;
    ++id;
  }
  
  // Multiple keys aren't supported yet
  x3d_assert(x3d_key == 1);
  
  
  key_map[id][0] = row;
  key_map[id][1] = col;
}

X3D_PLATFORM void x3d_read_keys() {
  int16 i;
  
  
  for(i = X3D_MAX_KEYS - 1; i >= 0; --i) {
    key_state <<= 1;
    
    if(key_map[i][0] != 0xFFFF)
      key_state |= _keytest(key_map[i][0], key_map[i][1]);
  }
}

_Bool x3d_key_down(X3D_Key key) {
  return key_state & key;
}

