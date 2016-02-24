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
#include "X3D_init.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"
#include "X3D_trig.h"

static SDL_Surface* window_surface;
static int16 screen_w;
static int16 screen_h;
static int16 screen_scale;
static _Bool record;
static int16 record_frame;
static char record_name[1024];
static _Bool virtual_window;

X3D_INTERNAL _Bool x3d_platform_screen_init(X3D_InitSettings* init) {
  x3d_log(X3D_INFO, "SDL init");
  
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    x3d_log(X3D_ERROR, "Failed to init SDL: %s", SDL_GetError());
    return X3D_FALSE;
  }
  
  screen_w = init->screen_w;
  screen_h = init->screen_h;
  screen_scale = init->screen_scale;
  virtual_window = init->flags & X3D_INIT_VIRTUAL_SCREEN;
  
  x3d_state->screen_manager.w = screen_w;
  x3d_state->screen_manager.h = screen_h;
  x3d_state->screen_manager.center.x = screen_w / 2;
  x3d_state->screen_manager.center.y = screen_h / 2;
  x3d_state->screen_manager.fov = init->fov;
  x3d_state->screen_manager.scale_x = div_int16_by_fp0x16(screen_w / 2, x3d_tan(init->fov / 2));
  x3d_state->screen_manager.scale_y = x3d_state->screen_manager.scale_x;  //screen_w * div_int16_by_fp0x16(screen_w / 2, x3d_tan(init->fov / 2)) / screen_h;
  
  x3d_log(X3D_INFO, "Create window (w=%d, h=%d, pix_scale=%d, render_scale=%d)",
          init->screen_w, init->screen_h, init->screen_scale, x3d_state->screen_manager.scale_x);
  
  if(!(init->flags & X3D_INIT_VIRTUAL_SCREEN)) {
    window_surface = SDL_SetVideoMode(
      init->screen_w * init->screen_scale,
      init->screen_h * init->screen_scale,
      32,
      SDL_SWSURFACE
    );
    
    if(!window_surface) {
      x3d_log(X3D_ERROR, "Failed to create window");
      return X3D_FALSE;
    }
    
    x3d_log(X3D_INFO, "Window created");
  }
  else {
    uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    window_surface = SDL_CreateRGBSurface(0, init->screen_w * init->screen_scale, init->screen_h * init->screen_scale, 32,
                                   rmask, gmask, bmask, amask);
    
    if(!window_surface) {
      x3d_log(X3D_ERROR, "Failed to create virtual window");
      return X3D_FALSE;
    }
    
    x3d_log(X3D_INFO, "Created virtual window");
  }
  
  record = X3D_FALSE;
  record_frame = 0;
  
  return X3D_TRUE;
  
}

X3D_INTERNAL void x3d_platform_screen_cleanup(void) {
  SDL_FreeSurface(window_surface);
  SDL_Quit();
}


#define PURPLE (16 | (16 << 10))

#define BPP 15

static uint32 map_color_to_uint32(X3D_Color color) {
#if BPP == 15
  const uint16 mask = (1 << 5) - 1;
  uint16 red = 255 * (color & mask) / 31;
  uint16 green = 255 * ((color >> 5) & mask) / 31;
  uint16 blue = 255 * ((color >> 10) & mask) / 31;
  
  return SDL_MapRGB(window_surface->format, red, green, blue);
#else
  const uint16 mask = (1 << 8) - 1;
  uint16 red = 255 * (color & mask) / 255;
  uint16 green = 255 * ((color >> 8) & mask) / 255;
  uint16 blue = 255 * ((color >> 16) & mask) / 255;
  
  return SDL_MapRGB(window_surface->format, red, green, blue);
#endif
}

static uint32 scale_color(X3D_Color c, uint16 scale) {
  uint8 r, g, b;
  
  x3d_color_to_rgb(c, &r, &g, &b);
  
  return SDL_MapRGB(window_surface->format, ((uint32)r * scale) >> 15, ((uint32)g * scale) >> 15, ((uint32)b * scale) >> 15);
}

extern X3D_Vex3D color_err;

uint16 scale_down(uint32 value, int16* error) {
  int16 v = (value >> 8) + *error;
  
  int16 new_v;
  
#if 0
  if(v > 255)
    v = 255;
  
  if(v < 0)
    v = 0;
#endif
  
  if((v & 7) < 4) {
    new_v = v & ~7;
  }
  else {
    new_v = (v & ~7) + 8;
  }
  
  *error = v - new_v;
  
  return new_v;
}

X3D_Vex3D_int16 color_err;

X3D_Color x3d_color_scale(uint32 r, uint32 g, uint32 b) {
  return x3d_rgb_to_color(
    scale_down((uint32)r, &color_err.x),
    scale_down((uint32)g, &color_err.y),
    scale_down((uint32)b, &color_err.z)
  );
}

void x3d_screen_draw_scanline_grad2(int16 y, int16 left, int16 right, X3D_Color c, fp0x16 scale_left, fp0x16 scale_right) {
  uint16 i; 
  
  int32 scale = (int32)scale_left << 16;
  int32 scale_slope = (((int32)scale_right - scale_left) << 16) / (right - left + 1);
  
  color_err.x = 0;
  color_err.y = 0;
  color_err.z = 0;
    
  uint8 r1, g1, b1;
  x3d_color_to_rgb(c, &r1, &g1, &b1);
  
  int16 rr1, gg1, bb1;
  rr1 = ((int32)r1 * scale_left) >> 15;
  gg1 = ((int32)g1 * scale_left) >> 15;
  bb1 = ((int32)b1 * scale_left) >> 15;
  
  color_err.x = 0;
  color_err.y = 0;
  color_err.z = 0;
    
  uint8 r2, g2, b2;
  x3d_color_to_rgb(c, &r2, &g2, &b2);
  
  int16 rr2, gg2, bb2;
  rr2 = ((int32)r2 * scale_right) >> 15;
  gg2 = ((int32)g2 * scale_right) >> 15;
  bb2 = ((int32)b2 * scale_right) >> 15;

  
  int16 r_slope = (((int32)rr2 - rr1) << 8) / (right - left + 1);
  int16 g_slope = (((int32)gg2 - gg1) << 8) / (right - left + 1);
  int16 b_slope = (((int32)bb2 - bb1) << 8) / (right - left + 1);

  uint16 r = rr1 << 8;
  uint16 g = gg1 << 8;
  uint16 b = bb1 << 8;
  
  for(i = left; i <= right; ++i) {
    ((uint32 *)window_surface->pixels)[y * window_surface->w + i] = map_color_to_uint32(x3d_color_scale(r, g, b));
    
    r += r_slope;
    g += g_slope;
    b += b_slope;
  }
}

void x3d_screen_draw_scanline_grad(int16 y, int16 left, int16 right, X3D_Color c, fp0x16 scale_left, fp0x16 scale_right, X3D_Color* color_tab) {
  uint16 i;
  
#if 1
  if(x3d_key_down(X3D_KEY_15)) {
    x3d_screen_draw_scanline_grad2(y, left, right, c, scale_left, scale_right);
    return;
  }
#endif

  int16 map[3][3] = {
    { 1, 8, 4 },
    { 7, 6, 3 },
    { 5, 2, 9 }
  };
  
  uint16 total_c = 32;
  uint16 scale_bits = 5;
  
  uint16 scale_slope = (scale_right - scale_left) / (right - left + 1);
  uint16 scale = scale_left;
  
  int32 err = 0;
  
  int32 mask = (1 << (16 - scale_bits)) - 1;
  int32 half = mask / 2;
  
  for(i = left; i <= right; ++i) {
    int32 val = scale + scale * map[i % 3][y % 3] / 10;
    
    int16 index = val >> (15 - scale_bits);
    
    if(index >= total_c)
      index = total_c - 1;
    
#if 0
    err = (int32)(val & mask);
    
    if((val & mask) >= half) {
      err = -(mask + 1 - (val & mask));
      ++index;
    }
    
#endif
    ((uint32 *)window_surface->pixels)[y * window_surface->w + i] = map_color_to_uint32(color_tab[index]);
    scale += scale_slope;
  }
  
}

uint16 scale_value_down(uint32 value, int16* error) {
  int16 v = (value >> 15) + *error;
  
  int16 lo = (v / 8) * 8;
  int16 hi = (v / 8) * 8 + 8;
  
  int16 new_v;
  
  if(v > 255)
    v = 255;
  
  if(v < 0)
    v = 0;
  
  if(abs(lo - v) < abs(hi - v)) {
    new_v = lo;
  }
  else {
    new_v = hi;
  }
  
  *error = v - new_v;
  
  if(x3d_key_down(X3D_KEY_15))
    return new_v;
  else
    return v;
}

void dither_pixel(X3D_Vex3D* err, int16 x, int16 w, int16 y, int16 h, uint32 val) {
  uint8 r, g, b;
  //SDL_GetRGB(val, &r, &g, &b);
  
  X3D_Vex3D error = err[x];
  
  X3D_Color color = x3d_rgb_to_color(
    scale_value_down((uint32)r, &error.x),
    scale_value_down((uint32)g, &error.y),
    scale_value_down((uint32)b, &error.z)
  );
  
  err[x].x += 5 * error.x / 16;
  err[x].y += 5 * error.y / 16;
  err[x].z += 5 * error.z / 16;
  
  if(x + 1 < w) {
    err[x + 1].x += 7 * error.x / 16;
    err[x + 1].y += 7 * error.y / 16;
    err[x + 1].z += 7 * error.z / 16;
    
    if(y + 1 < h) {
    }
  }
}

void dither() {
  int16 w = screen_w;
  int16 h = screen_h;
  
  X3D_Vex3D err[w];
  
  int16 i;
  for(i = 0; i < w; ++i) {
    err[i].x = 0;
    err[i].y = 0;
    err[i].z = 0;
  }
  
  int16 x, y;
  
  for(y = 0; y < h; ++y) {
    for(x = 0; x < w; ++x) {
      
    }
  }
}
 
void x3d_screen_flip() {
  if(record) {
    char str[1100];
    sprintf(str, "%s%05d.bmp", record_name, record_frame);
    
    if((record_frame % 4) == 0) {
      if(SDL_SaveBMP(window_surface, str) != 0) {
        x3d_log(X3D_ERROR, "Error saving frame: %s\n", SDL_GetError());
        record = X3D_FALSE;
      }
      else {
        printf("Saved frame %s\n", str);
      }
    }
    
    ++record_frame;
  }
  
  if(!virtual_window)
    SDL_Flip(window_surface);
}

void* x3d_screen_get_internal(void) {
  return window_surface;
}

void x3d_screen_clear(X3D_Color color) {
  SDL_FillRect(window_surface, NULL, map_color_to_uint32(color));
}

void x3d_screen_draw_pix(int16 x, int16 y, X3D_Color color) {
  uint32 c = map_color_to_uint32(color);
  
  if(x < 0 || x >= screen_w || y < 0 || y >= screen_h)
    return;
  
  for(int32 i = 0; i < screen_scale; ++i) {
    for(int32 d = 0; d < screen_scale; ++d) {
      int32 xx = x * screen_scale + d;
      int32 yy = y * screen_scale + i;
      
      ((uint32 *)window_surface->pixels)[yy * window_surface->w + xx] = c;
    }
  }
}

void x3d_screen_draw_line(int16 x0, int16 y0, int16 x1, int16 y1, X3D_Color color) {
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
#if BPP == 15
  return (31 * (uint16)r / 255) +
    ((31 * (uint16)g / 255) << 5) +
    ((31 * (uint16)b / 255) << 10);
#else
  return (255 * (uint32)r / 255) +
    ((255 * (uint32)g / 255) << 8) +
    ((255 * (uint32)b / 255) << 16);
#endif
}

void x3d_color_to_rgb(X3D_Color color, uint8* r, uint8* g, uint8* b) {
#if BPP == 15
  const uint16 mask = (1 << 5) - 1;
  *r = 255 * (color & mask) / 31;
  *g = 255 * ((color >> 5) & mask) / 31;
  *b = 255 * ((color >> 10) & mask) / 31;
#else
  const uint16 mask = (1 << 8) - 1;
  *r = 255 * (color & mask) / 255;
  *g = 255 * ((color >> 8) & mask) / 255;
  *b = 255 * ((color >> 16) & mask) / 255;
#endif
}

void x3d_screen_begin_record(const char* name) {
  strcpy(record_name, name);
  record = X3D_TRUE;
  record_frame = 0;
}

void x3d_screen_record_end(void) {
  record = X3D_FALSE;
}
