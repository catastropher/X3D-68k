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

#include "X3D.h"

void x3dplatform_key_map_pc(uint32 sdl_key_a, uint32 sdl_key_b, uint16 x3d_key);

int main() {
  X3D_InitSettings settings = {
    .screen_w = 240,        // Screen width
    .screen_h = 128,        // Screen height
    .screen_scale = 3       // Screen scale
  };
  
  x3d_init(&settings);
  x3dplatform_key_map_pc(SDLK_LSHIFT, SDLK_RETURN, 0);
  x3d_screen_flip();
}

