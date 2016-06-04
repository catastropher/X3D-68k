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

#include <SDL/SDL.h>

#include "X3D_common.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"

void x3d_renderer_draw_hud(void) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  if(renderman->render_hud_callback)
    renderman->render_hud_callback();
  
  x3d_renderer_draw_fps();
  x3d_renderer_draw_crosshair();
}

void x3d_renderer_draw_crosshair(void) {
  X3D_ScreenManager* screenman = x3d_screenmanager_get();
  int16 cx = screenman->center.x;
  int16 cy = screenman->center.y;
  
  x3d_screen_draw_pix(cx, cy - 1, 0xFFFF);
  x3d_screen_draw_pix(cx, cy + 1, 0xFFFF);
  x3d_screen_draw_pix(cx - 1, cy, 0xFFFF);
  x3d_screen_draw_pix(cx + 1, cy, 0xFFFF);
}

void x3d_renderer_draw_fps(void) {
  static uint32 start = 0;
  static uint16 frames = 0;
  static int32 fps = 0;

  if(++frames == 10) {
    int32 time = (SDL_GetTicks() - start);
    if(time != 0)
      fps = 1000000 / time;
    else
      fps = 100000;
    
    frames = 0;
    
    start = SDL_GetTicks();
  }
  
  x3d_screen_draw_uint32(fps, 0, 0, 31);
}


