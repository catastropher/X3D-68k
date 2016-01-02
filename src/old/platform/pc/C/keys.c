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
//
//
//
// keys.c

#include <SDL2/SDL.h>

#include "common/X3D_int.h"
#include "common/X3D_interface.h"

//#include "platform/debug/X3D_platform_log.h"
#include "platform/X3D_platform.h"


#define SDL_TOTAL_KEYS 322
#define X3D_KEY_NONE 0xFFFF

static _Bool keystate[SDL_TOTAL_KEYS];

typedef uint32 X3D_KeyState;

// Reset mask
// SDL key 1
// SDK key 2

typedef struct X3D_Key {
  X3D_KeyState mask;
  uint32 sdl_key_a;
  uint32 sdl_key_b;
} X3D_Key;

void x3dplatform_key_init() {
  for(int16 i = 0; i < SDL_TOTAL_KEYS; ++i)
    keystate[i] = X3D_FALSE;
}

void x3dplatform_key_map_pc(uint32 sdl_key_a, uint32 sdl_key_b, uint16 x3d_key) {
  // Swap keys if given in wrong order
  if(sdl_key_a == X3D_KEY_NONE) {
    uint16 temp = sdl_key_a;
    sdl_key_a = sdl_key_b;
    sdl_key_b = temp;
  }
  
  // If we're creating a key combo, we need to create a clear mask for the key
  // state. Thus, if they press CTRL+A, it will only register as CTRL+A, not
  // CTRL and A.
  
  char name[128];
  
  if(sdl_key_a == X3D_KEY_NONE) {
    x3dplatform_log(X3D_INFO, "Reset keymap for key %d\n", x3d_key);
  }
  else {
    strcpy(name, SDL_GetKeyName(sdl_key_a));
    
    if(sdl_key_b != X3D_KEY_NONE) {
      strcat(name, "+");
      strcat(name, SDL_GetKeyName(sdl_key_b));
    }
    
    x3dplatform_log(X3D_INFO, "Mapped %s to key %d\n", name, x3d_key);
  }
  
}


_Bool x3dplatform_key_down();







