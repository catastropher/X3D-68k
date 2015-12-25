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
// screen.c -> PC implentation of the screen interface

#include <SDL2/SDL.h>

#include "X3D.h"

static SDL_Window* window;
static SDL_Surface* window_surface;
static int16 screen_w;
static int16 screen_h;

X3D_INTERNAL _Bool x3dplatform_screen_init(X3D_InitSettings* init) {
  x3dplatform_log(X3D_INFO, "SDL init");
  
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    x3dplatform_log(X3D_ERROR, "Failed to init SDL");
    return X3D_FALSE;
  }
  
  x3dplatform_log(X3D_INFO, "Create window");
  
  window = SDL_CreateWindow(
    "X3D",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    init->screen_w,
    init->screen_h,
    SDL_WINDOW_OPENGL
  );
  
  if(!window) {
    x3dplatform_log(X3D_ERROR, "Failed to create window");
    return X3D_FALSE;
  }
  
  window_surface = SDL_GetWindowSurface(window);
  screen_w = init->screen_w;
  screen_h = init->screen_h;
  
  return X3D_TRUE;
  
}


#define PURPLE (16 | (16 << 10))

static uint32 map_color_to_uint32(X3D_Color color) {
  const uint16 mask = (1 << 5) - 1;
  printf("Mask: %d\n", mask);
  uint16 red = 255 * (color & mask) / 32;
  uint16 green = 255 * ((color >> 5) & mask) / 32;
  uint16 blue = 255 * ((color >> 10) & mask) / 32;
  
  return SDL_MapRGB(window_surface->format, red, green, blue);
}


X3D_IMPLEMENTATION void x3dplatform_screen_flip() {
  SDL_UpdateWindowSurface(window);
}

X3D_IMPLEMENTATION void x3dplatform_screen_clear(X3D_Color color) {
  SDL_FillRect(window_surface, NULL, map_color_to_uint32(PURPLE));// map_color_to_uint32(color));
}

X3D_IMPLEMENTATION void x3dplatform_draw_pix(int16 x, int16 y, X3D_Color color) {
  ((uint32 *)window_surface->pixels)[y * screen_w + x] = map_color_to_uint32(color);
}

