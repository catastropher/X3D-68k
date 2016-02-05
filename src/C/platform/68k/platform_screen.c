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
#include "X3D_screen.h"
#include "X3D_enginestate.h"
#include "X3D_trig.h"

static int16 screen_w;
static int16 screen_h;
static int16 screen_scale;
static _Bool record;
static int16 record_frame;

uint8* screen;

X3D_INTERNAL _Bool x3d_platform_screen_init(X3D_InitSettings* init) {
  x3d_log(X3D_INFO, "SDL init");
  
  screen_w = init->screen_w;
  screen_h = init->screen_h;
  screen_scale = init->screen_scale;
  
  x3d_state->screen_manager.w = screen_w;
  x3d_state->screen_manager.h = screen_h;
  x3d_state->screen_manager.center.x = screen_w / 2;
  x3d_state->screen_manager.center.y = screen_h / 2;
  x3d_state->screen_manager.fov = init->fov;
  x3d_state->screen_manager.scale_x = div_int16_by_fp0x16(screen_w / 2, x3d_tan(init->fov / 2));
  x3d_state->screen_manager.scale_y = x3d_state->screen_manager.scale_x;  //screen_w * div_int16_by_fp0x16(screen_w / 2, x3d_tan(init->fov / 2)) / screen_h;
  
  x3d_log(X3D_INFO, "Create window (w=%d, h=%d, pix_scale=%d, render_scale=%d)",
          init->screen_w, init->screen_h, init->screen_scale, x3d_state->screen_manager.scale_x);
  
  record = X3D_FALSE;
  record_frame = 0;
  
  x3d_log(X3D_INFO, "Window created");
  
  screen = malloc(LCD_SIZE);
  PortSet(screen, 240 - 1, 128 - 1);
  
  return X3D_TRUE;
  
}

X3D_INTERNAL void x3d_platform_screen_cleanup(void) {
}


#define PURPLE (16 | (16 << 10))

static uint32 map_color_to_uint32(X3D_Color color) {
  return color != 0 ? 1 : 0;
}


void x3d_screen_flip() {
  LCD_restore(screen);
}

void x3d_screen_clear(X3D_Color color) {
  clrscr();
}

void x3d_screen_draw_pix(int16 x, int16 y, X3D_Color color) {
  DrawPix(x, y, A_NORMAL);
}

void x3d_screen_draw_line(int16 x0, int16 y0, int16 x1, int16 y1, X3D_Color color) {
  DrawLine(x0, y0, x1, y1, A_NORMAL);
  return;
  
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  for(;;){
    x3d_screen_draw_pix(x0, y0, color);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void x3d_screen_draw_line_grad(int16 x0, int16 y0, int16 x1, int16 y1, X3D_Color color0, X3D_Color color1) {
  x3d_screen_draw_line(x0, y0, x1, y1, 1);
  return;
  
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  uint8 r0, g0, b0;
  x3d_color_to_rgb(color0, &r0, &g0, &b0);
  
  uint8 r1, g1, b1;
  x3d_color_to_rgb(color1, &r1, &g1, &b1);
  
  int16 step;
  
  _Bool step_x;
  
  if(dx > dy) {
    step = dx;
    step_x = X3D_TRUE;
  }
  else {
    step = dy;
    step_x = X3D_FALSE;
  }
  
  if(step == 0)
    return;
  
  int32 step_r = (((int32)r1 - r0) * 65536L) / step;
  int32 step_g = (((int32)g1 - g0) * 65536L) / step;
  int32 step_b = (((int32)b1 - b0) * 65536L) / step;
  
  int32 r = ((int32)r0) << 16;
  int32 g = ((int32)g0) << 16;
  int32 b = ((int32)b0) << 16;
  
  _Bool update = X3D_FALSE;
  
  X3D_Color color = color1;
  
  for(;;){
    x3d_screen_draw_pix(x0, y0, color);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) {
      err -= dy;
      x0 += sx;
      
      if(step_x) {
        update = X3D_TRUE;
      }
    }
    
    if (e2 < dy) {
      err += dx;
      y0 += sy;
      
      if(!step_x) {
        update = X3D_TRUE;
      }
    }
    
    if(update) {
      r += step_r;
      g += step_g;
      b += step_b;
      
      color = x3d_rgb_to_color(r >> 16, g >> 16, b >> 16);
      update = X3D_FALSE;
    }
  }
}

X3D_Color x3d_rgb_to_color(uint8 r, uint8 g, uint8 b) {
  return (31 * (uint16)r / 255) +
    ((31 * (uint16)g / 255) << 5) +
    ((31 * (uint16)b / 255) << 10);
}

void x3d_color_to_rgb(X3D_Color color, uint8* r, uint8* g, uint8* b) {
  const uint16 mask = (1 << 5) - 1;
  *r = 255 * (color & mask) / 31;
  *g = 255 * ((color >> 5) & mask) / 31;
  *b = 255 * ((color >> 10) & mask) / 31;
}

void x3d_screen_begin_record(const char* name) {
}

void x3d_screen_record_end(void) {
}
