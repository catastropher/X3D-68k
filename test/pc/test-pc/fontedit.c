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

#include "X3D.h"
#include <SDL2/SDL.h>

enum {
  KEY_LEFT = X3D_KEY_0,
  KEY_RIGHT = X3D_KEY_1,
  KEY_UP = X3D_KEY_2,
  KEY_DOWN = X3D_KEY_3,
  KEY_ENTER = X3D_KEY_4
};

X3D_Vex2D cursor;

// TODO: figure out screen size
int16 height = 7;
int16 width = 5;
int16 scale;

void font_editor_key_handler(void) {
  x3d_read_keys();
  
  if (x3d_key_down(KEY_LEFT)) {
    cursor.x = (cursor.x != 0 ? cursor.x - 1 : width - 1);
    
    while (x3d_key_down(KEY_LEFT)) x3d_read_keys();
  }
  
  if (x3d_key_down(KEY_RIGHT)) {
    cursor.x = (cursor.x != width - 1 ? cursor.x + 1 : 0);
    
    while (x3d_key_down(KEY_RIGHT)) x3d_read_keys();
  }
  
  if (x3d_key_down(KEY_UP)) {
    cursor.y = (cursor.y != 0 ? cursor.y - 1 : height - 1);
    
    while (x3d_key_down(KEY_UP)) x3d_read_keys();
  }
  
  if (x3d_key_down(KEY_DOWN)) {
    cursor.y = (cursor.y != height - 1 ? cursor.y + 1 : 0);
    
    while (x3d_key_down(KEY_DOWN)) x3d_read_keys();
  }
  
  // TODO: Enter button behavior
  if (x3d_key_down(KEY_ENTER)) {
    
  }
}

void font_editor(void) {
  X3D_InitSettings init = {
    .screen_w = 640,
    .screen_h = 480,
    .screen_scale = 1,
    .fullscreen = X3D_FALSE,
    .fov = ANG_60
  };
  
  x3d_init(&init);
  
  // Set up key mapping
  x3d_key_map_pc(KEY_UP, 'w');
  x3d_key_map_pc(KEY_DOWN, 's');
  x3d_key_map_pc(KEY_LEFT, 'a');
  x3d_key_map_pc(KEY_RIGHT, 'd');
  x3d_key_map_pc(KEY_ENTER, SDLK_RETURN);
  
  x3d_keymanager_set_callback(font_editor_key_handler);
  x3d_cleanup();
}



