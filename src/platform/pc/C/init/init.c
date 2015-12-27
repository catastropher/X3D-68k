/* This file is part of X3D.
 * 
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <SDL2/SDL.h>


#include "X3D.h"

static SDL_Window* window;


X3D_INTERNAL _Bool x3dplatform_screen_init(X3D_InitSettings* init);

X3D_IMPLEMENTATION _Bool x3dplatform_init(struct X3D_InitSettings* init) {
  x3dplatform_screen_init(init);
  
  x3dplatform_screen_clear(0);
  x3dplatform_screen_flip();
  
  SDL_Delay(10000);
  
  SDL_Quit();
}















