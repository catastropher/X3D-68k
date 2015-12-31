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
// test.c -> test for PC

#include <SDL2/SDL.h>

#include "X3D_common.h"
#include "X3D_log.h"
#include "X3D_init.h"
#include "X3D_screen.h"

int main() {
  X3D_InitSettings init = {
    .screen_w = 240,
    .screen_h = 128,
    .screen_scale = 3,
    .fullscreen = X3D_FALSE
  };
  
  
  x3d_init(&init);
  x3d_screen_clear(31);
  x3d_screen_draw_line(20, 20, 100, 50, 0);
  x3d_screen_flip();
  
  SDL_Delay(2000);
  
  SDL_Quit();
}

