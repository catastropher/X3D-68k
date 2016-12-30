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

#ifdef X3D_USE_SDL1
#include <SDL/SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "X3D_common.h"
#include "X3D_init.h"
#include "X3D_keys.h"
#include "X3D_assert.h"

// Total number of keys that SDL defines
#define SDL_TOTAL_KEYS 322



// Holds whether each key is currently being pressed
static _Bool sdl_keys[SDL_TOTAL_KEYS];

// The max X3D key that has been set (will be used to determine how many bytes)
//    of the keystate need to be transferred with multiplayer.
//static int16 max_key;

// Holds which X3D keys are currently being pressed
static uint32 key_state;

// Holds which keys have been pressed since the last time checked
static uint32 key_pressed_state;

// Maps an SDL key to an X3D key
static int32 key_map[X3D_MAX_KEYS];


_Bool x3d_pc_key_down(int32 key) {
    return sdl_keys[key & (~SDLK_SCANCODE_MASK)];
}

void x3d_pc_mouse_state(_Bool* left, _Bool* right, int16* x, int16* y) {
  int xx, yy;
  
  uint8 buttons = SDL_GetMouseState(&xx, &yy);
  
  *x = xx;
  *y = yy;
  
  *left = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
  *right = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
}

X3D_INTERNAL void x3d_platform_keys_init(X3D_InitSettings* settings) {
  x3d_log(X3D_INFO, "Key mapper init\n");
  
  for(int16 i = 0; i < SDL_TOTAL_KEYS; ++i)
    sdl_keys[i] = X3D_FALSE;
  
  for(int16 i = 0; i < X3D_MAX_KEYS; ++i) {
    key_map[i] = X3D_KEY_NONE;
  }
  
  key_state = 0;
  key_pressed_state = 0;
}

void x3d_key_map_pc(uint32 x3d_key, int32 sdl_key) {
  int id = 0;
  
  x3d_assert(x3d_key != 0);
  
  while((x3d_key & 1) == 0) {
    x3d_key >>= 1;
    ++id;
  }
  
  // Multiple keys aren't supported yet
  x3d_assert(x3d_key == 1);
  
  
#ifdef X3D_USE_SDL1
  key_map[id] = sdl_key;
#else
  key_map[id] = sdl_key & (~SDLK_SCANCODE_MASK);
#endif
  
  if(sdl_key != X3D_KEY_NONE)
    x3d_log(X3D_INFO, "Mapped key '%s' to 'X3D_KEY_%d'", SDL_GetKeyName(key_map[id]), id);
  else
    x3d_log(X3D_INFO, "Reset key 'X3D_KEY_%d'", id);
}

void x3d_pc_send_event(void* ev) {
    SDL_Event event = *(SDL_Event*)ev;
    
    switch(event.type) {
        case SDL_KEYDOWN:
            //printf("Key: %d\n", event.key.keysym.sym);
            #ifdef X3D_USE_SDL1
            sdl_keys[event.key.keysym.sym] = X3D_TRUE;
            #else
            sdl_keys[event.key.keysym.sym & (~SDLK_SCANCODE_MASK)] = X3D_TRUE;
            #endif
            break;
            
        case SDL_KEYUP:
            #ifdef X3D_USE_SDL1
            sdl_keys[event.key.keysym.sym] = X3D_FALSE;
            #else
            sdl_keys[event.key.keysym.sym & (~SDLK_SCANCODE_MASK)] = X3D_FALSE;
            #endif
            break;
            
        default:
            break;
    }
}

X3D_PLATFORM void x3d_read_keys() {
  int16 i;
  SDL_Event event;
  
  // Process the SDL events
  /// @todo Add SDL_QUIT
  while(SDL_PollEvent(&event)) {
      x3d_pc_send_event(&event);
  }
  
  // Update the X3D keystate based on which keys are being pressed
  key_state = 0;
  
  for(i = X3D_MAX_KEYS - 1; i >= 0; --i) {
    key_state <<= 1;
    
    if(key_map[i] != X3D_KEY_NONE)
      key_state |= sdl_keys[key_map[i]];
  }
}

_Bool x3d_key_down(uint32 key) {
  return key_state & key;
}

