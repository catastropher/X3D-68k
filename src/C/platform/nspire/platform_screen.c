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
#include "render/X3D_texture.h"

static SDL_Surface* window_surface;
static int16 screen_w;
static int16 screen_h;
static int16 screen_scale;
static _Bool record;
static int16 record_frame;
static char record_name[1024];

X3D_INTERNAL _Bool x3d_platform_screen_init(X3D_InitSettings* init) {
  x3d_log(X3D_INFO, "SDL init");
  
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    x3d_log(X3D_ERROR, "Failed to init SDL: %s", SDL_GetError());
    return X3D_FALSE;
  }
  
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
  
  window_surface = SDL_SetVideoMode(
    init->screen_w * init->screen_scale,
    init->screen_h * init->screen_scale,
    16,
    SDL_SWSURFACE
  );
  
  if(!window_surface) {
    x3d_log(X3D_ERROR, "Failed to create window");
    return X3D_FALSE;
  }
  
  record = X3D_FALSE;
  record_frame = 0;
  
  x3d_log(X3D_INFO, "Window created");
  
  return X3D_TRUE;
  
}

X3D_INTERNAL void x3d_platform_screen_cleanup(void) {
  SDL_FreeSurface(window_surface);
  SDL_Quit();
}


#define PURPLE (16 | (16 << 10))

static uint32 map_color_to_uint32(X3D_Color color) {
  const uint16 mask = (1 << 5) - 1;
  uint16 red = 255 * (color & mask) / 31;
  uint16 green = 255 * ((color >> 5) & mask) / 31;
  uint16 blue = 255 * ((color >> 10) & mask) / 31;
  
  return SDL_MapRGB(window_surface->format, red, green, blue);
}


void x3d_screen_flip() {
  SDL_Flip(window_surface);
}

void x3d_screen_clear(X3D_Color color) {
  SDL_FillRect(window_surface, NULL, map_color_to_uint32(color));
}

inline void x3d_screen_draw_pix(int16 x, int16 y, X3D_Color color) {
  uint32 c = map_color_to_uint32(color);
  
  if(x < 0 || x >= screen_w || y < 0 || y >= screen_h)
    return;
  
  for(int32 i = 0; i < 1; ++i) {
    for(int32 d = 0; d < 1; ++d) {
      int32 xx = x * screen_scale + d;
      int32 yy = y * screen_scale + i;
      
      ((uint16 *)window_surface->pixels)[yy * window_surface->w + xx] = c;
    }
  }
}

void x3d_screen_draw_line(int16 x0, int16 y0, int16 x1, int16 y1, X3D_Color color) {
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  if(y0 == y1) {
    int16 start = X3D_MIN(x0, x1);
    int16 end = X3D_MAX(x0, x1);
    
    start = X3D_MAX(start, 0);
    end = X3D_MIN(end, screen_w - 1);
    
    int16 i;
    
    uint16 c = map_color_to_uint32(color);
    
    for(i = start; i <= end; ++i) {
      ((uint16 *)window_surface->pixels)[y0 * window_surface->w + i] = c;
    }
    return;
  }
  
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
  strcpy(record_name, name);
  record = X3D_TRUE;
  record_frame = 0;
}

void x3d_screen_record_end(void) {
  record = X3D_FALSE;
}

X3D_Vex3D color_err;

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

X3D_Color x3d_color_scale(uint32 r, uint32 g, uint32 b) {
  return x3d_rgb_to_color(
    scale_down((uint32)r, &color_err.x),
    scale_down((uint32)g, &color_err.y),
    scale_down((uint32)b, &color_err.z)
  );
}

void x3d_screen_zbuf_clear(void) {
  uint32 i;
  
  for(i = 0; i < (int32)screen_w * screen_h; ++i)
    x3d_rendermanager_get()->zbuf[i] = 0;
}

uint32 x3d_color_to_internal(X3D_Color c) {
  return map_color_to_uint32(c);
}

void x3d_screen_draw_scanline_grad(int16 y, int16 left, int16 right, X3D_Color c, fp0x16 scale_left, fp0x16 scale_right, X3D_Color* color_tab, int16 z) {
uint16 i;
  
#if 0
  if(x3d_key_down(X3D_KEY_15)) {
    x3d_screen_draw_scanline_grad2(y, left, right, c, scale_left, scale_right);
    return;
  }
#endif

#if 1
  int16 map[3][3] = {
    { 1, 8, 4 },
    { 7, 6, 3 },
    { 5, 2, 9 }
  };
#endif
  
  
#if 0
  int16 map[4][4] = {
    { 1, 9, 3, 11 },
    { 13, 5, 15, 7 },
    { 4, 12, 2, 10 },
    { 16, 8, 14, 16 }
  };
#endif
  
#if 0
  int16 map[8][8] = {
    { 1, 49, 13, 61, 4, 52, 16, 64 },
    { 33, 17, 45, 29, 36, 20, 48, 32 },
    { 9, 57, 5, 53, 12, 60, 8, 56 },
    { 41, 25, 37, 21, 44, 28, 40, 24 },
    { 3, 51, 15, 63, 2, 50, 14, 62 },
    { 25, 19, 47, 31, 34, 18, 46, 30 },
    { 11, 59, 7, 55, 10, 58, 6, 54 },
    { 43, 27, 39, 23, 42, 26, 38, 22 }
  };
#endif
  
  uint16 total_c = 64;
  uint16 scale_bits = 6;
  
  uint16 scale_slope = (scale_right - scale_left) / (right - left + 1);
  uint16 scale = scale_left;
  
  int32 err = 0;
  
  int32 mask = (1 << (16 - scale_bits)) - 1;
  int32 half = mask / 2;
  
  for(i = left; i <= right; ++i) {
    int32 val;
    
    if(!x3d_key_down(X3D_KEY_15))
      val = scale + scale * map[i % 3][y % 3] / 10;
    else
      val = scale;
    
    
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
    ((uint16 *)window_surface->pixels)[y * window_surface->w + i] = color_tab[index];
    scale += scale_slope;
  }
}

X3D_Texture panel_tex;
X3D_Texture brick_tex;
X3D_Texture floor_panel_tex;
X3D_Texture* global_texture = &panel_tex;

void x3d_set_texture(int16 id) {
  if(id == 0)       global_texture = &panel_tex;
  else if(id == 1)  global_texture = &brick_tex;
  else if(id == 2)  global_texture = &floor_panel_tex;
}

_Bool x3d_platform_screen_load_texture(X3D_Texture* tex, const char* file) {
  return X3D_FALSE;
}

void x3d_screen_draw_scanline_texture(X3D_Span* span, int16 y) {
  int16 dx = span->right.x - span->left.x;
  //fp16x16 u_slope = 0;
  //fp16x16 v_slope = 0;
  
  x3d_fix_slope u_slope;
  x3d_fix_slope v_slope;
  x3d_fix_slope u;
  x3d_fix_slope v;
  
  fp16x16 z_slope = 0;
  //fp16x16 u = (int32)span->left.u;
  //fp16x16 v = (int32)span->left.v;
  fp16x16 z = (int32)span->left.z;
  
  if(dx != 0) {
    x3d_fix_slope_init(&u_slope, span->left.u, span->right.u, dx);
    x3d_fix_slope_same_shift(&u, &u_slope, span->left.u);
    
    x3d_fix_slope_init(&v_slope, span->left.v, span->right.v, dx);
    x3d_fix_slope_same_shift(&v, &v_slope, span->left.v);
    
    //u_slope = x3d_val_slope2(span->right.u - span->left.u, dx);
    //v_slope = x3d_val_slope2(span->right.v - span->left.v, dx);
    z_slope = x3d_val_slope2(span->right.z - span->left.z, dx);
  }
  
  int16* z_buf = x3d_rendermanager_get()->zbuf;
  
  uint16 i;
  for(i = span->left.x; i <= span->right.x; ++i) {
    int32 zz = z >> 7;
    
    if(zz <= 0) zz = 0x7FFF;
    
    int16 uu = (x3d_fix_slope_val(&u) / zz);   //(((u / zz) >> 1) * 128) >> 15;
    int16 vv = (x3d_fix_slope_val(&v) / zz);//(((v / zz) >> 1) * 128) >> 15;
    
    //x3d_log(X3D_INFO, "u: %d, v: %d", uu, vv);
    
    //uu /= 16;
    //vv /= 16;
    
    //int16 uu = (((u >> 16) * zz) * 192) >> 15;
    //int16 vv = (((v >> 16) * zz) * 192) >> 15;
    
    
    zz >>= 8;
    
    X3D_Color c = x3d_texture_get_texel(global_texture, uu, vv);
    
    if(zz >= z_buf[y * window_surface->w + i]) {
      ((uint16 *)window_surface->pixels)[y * window_surface->w + i] = map_color_to_uint32(c);
      z_buf[y * window_surface->w + i] = zz;
    }
    
    //u += u_slope;
    //v += v_slope;
    x3d_fix_slope_add(&u, &u_slope);
    x3d_fix_slope_add(&v, &v_slope);
    
    
    z += z_slope;
  }
}

