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

#include "X3D_common.h"
#include "X3D_init.h"
#include "X3D_keys.h"

// Total number of keys that SDL defines
#define SDL_TOTAL_KEYS 322



// Holds whether each key is currently being pressed
static _Bool sdl_keys[SDL_TOTAL_KEYS];

// The max X3D key that has been set (will be used to determine how many bytes)
//    of the keystate need to be transferred with multiplayer.
static int16 max_key;

// Holds which X3D keys are currently being pressed
static uint32 key_state;

// Maps an SDL key to an X3D key
static int32 key_map[X3D_MAX_KEYS];


void x3d_platform_keys_init(X3D_InitSettings* settings) {
  for(int i = 0; i < SDL_TOTAL_KEYS; ++i)
    sdl_keys[i] = X3D_FALSE;
  
  key_state = 0;
}

void x3d_map_key() {
  
}




