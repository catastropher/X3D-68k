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
#include <stdlib.h>
#include <stdio.h>

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
static _Bool virtual_window;

X3D_Texture panel_tex;
X3D_Texture brick_tex;
X3D_Texture floor_panel_tex;
X3D_Texture cube_tex;
X3D_Texture aperture_tex;

X3D_Texture* global_texture = &brick_tex;

void x3d_set_texture(int16 id) {
  if(id == 0)       global_texture = &panel_tex;
  else if(id == 1)  global_texture = &brick_tex;
  else if(id == 2)  global_texture = &floor_panel_tex;
  else if(id == 3)  global_texture = &cube_tex;
  else if(id == 4)  global_texture = &aperture_tex;
}

X3D_INTERNAL _Bool x3d_platform_screen_init(X3D_InitSettings* init) {
  x3d_log(X3D_INFO, "SDL init");
  
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    x3d_log(X3D_ERROR, "Failed to init SDL: %s", SDL_GetError());
    return X3D_FALSE;
  }
  
  if(!x3d_texture_load_from_file(&aperture_tex, "aperture.bmp")) {
    x3d_log(X3D_ERROR, "Failed to load cube texture: %s", SDL_GetError());
  }
  
#if 0
  if(!x3d_texture_load_from_file(&cube_tex, "xiao.bmp")) {
    x3d_log(X3D_ERROR, "Failed to load cube texture: %s", SDL_GetError());
  }
  
  if(!x3d_texture_load_from_file(&panel_tex, "panel.bmp")) {
    x3d_log(X3D_ERROR, "Failed to load wood texture: %s", SDL_GetError());
  }

  if(!x3d_texture_load_from_file(&brick_tex, "wood2.bmp")) {
    x3d_log(X3D_ERROR, "Failed to load brick texture: %s", SDL_GetError());
  }
  
  if(!x3d_texture_load_from_file(&floor_panel_tex, "floor_panel2.bmp")) {
    x3d_log(X3D_ERROR, "Failed to load floor panel texture: %s", SDL_GetError());
  }
  
  if(!x3d_texture_load_from_file(&cube_tex, "cube.bmp")) {
    x3d_log(X3D_ERROR, "Failed to load cube texture: %s", SDL_GetError());
  }
  
  FILE* f = fopen("/home/michael/code/X3D-68k/test/pc/test-pc/texturepack.c", "wb");
  
  fprintf(f, "#include \"X3D_common.h\"\n\n");
  
  x3d_texture_to_array(&panel_tex, f, "panel_tex");
  x3d_texture_to_array(&brick_tex, f, "wood_tex");
  x3d_texture_to_array(&floor_panel_tex, f, "floor_panel_tex");
  x3d_texture_to_array(&cube_tex, f, "cube_tex");
  fclose(f);
  
  exit(0);
#endif
  
#if 0
  SDL_Surface* s = SDL_LoadBMP("piano.bmp");
  x3d_gray_dither(s);
  SDL_SaveBMP(s, "piano_bw.bmp");
  exit(0);
#endif

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

    window_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, init->screen_w * init->screen_scale, init->screen_h * init->screen_scale, 32,
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

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

_Bool x3d_platform_screen_load_texture(X3D_Texture* tex, const char* file) {
  x3d_log(X3D_INFO, "File: %s", file);
  SDL_Surface* s = SDL_LoadBMP(file);
  
  if(!s)
    return X3D_FALSE;
  
  tex->texel = malloc(s->w * s->h * sizeof(X3D_Color));
  
  if(!tex->texel) {
    SDL_FreeSurface(s);
    return X3D_FALSE;
  }
  
  tex->w = s->w;
  tex->h = s->h;
  tex->mask = tex->w - 1;
  
  uint16 i, d;
  for(i = 0; i < s->h; ++i) {
    for(d = 0; d < s->w; ++d) {
      uint32 pix = getpixel(s, d, i);
      uint8 r, g, b;
      
      SDL_GetRGB(pix, s->format, &r, &g, &b);
      
      x3d_texture_set_texel(tex, d, i, x3d_rgb_to_color(r, g, b));
    }
  }
  
  SDL_FreeSurface(s);
  
  return X3D_TRUE;
}


void x3d_gray_dither(SDL_Surface* s) {
  int32 x, y;
  
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
  
  for(y = 0; y < s->h; ++y) {
    for(x = 0; x < s->w; ++x) {
      uint32 pix = getpixel(s, x, y);
      uint8 r, g, b;
      SDL_GetRGB(pix, s->format, &r, &g, &b);
      
      float in = .299 * r + .587 * g + .114 * b;
      
      int16 val = in + in * map[y % 8][x % 8] / 63;
      
      if(val >= 192)
        val = 255;
      else
        val = 0;
      
      putpixel(s, x, y, SDL_MapRGB(s->format, val, val, val));
    }
  }
}

X3D_Color x3d_texture_get_pix(X3D_Texture* tex, int16 u, int16 v) {
#if 0
  uint8 r, g, b;

  return rand();
  
  SDL_Surface* s = tex->surface;
  
  if(u < 0 || u >= s->w || v < 0 || v >= s->h)
    return 0;
  
  //x3d_log(X3D_INFO, "U: %d, v: %d : %d, %d - bpp: %d", u, v, s->w, s->h, s->format->BytesPerPixel);
  
  SDL_GetRGB(getpixel(s, u, v), s->format, &r, &g, &b);
  
  return x3d_rgb_to_color(r, g, b);
#endif
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

extern int16 render_mode;

void x3d_screen_zbuf_clear(void) {
  uint32 i;
  
  for(i = 0; i < (int32)screen_w * screen_h; ++i)
    x3d_rendermanager_get()->zbuf[i] = 0;
}

void x3d_screen_zbuf_visualize(void) {
  X3D_Color c;
  uint16 x, y;
  
  for(y = 0; y < screen_h; ++y) {
    for(x = 0; x < screen_w; ++x) {
      int16 s = (int32)x3d_rendermanager_get()->zbuf[y * screen_w + x] * 255 * 32 / 0x7FFF;
      
      s = X3D_MIN(s, 255);
      
      x3d_screen_draw_pix(x, y, x3d_rgb_to_color(s, s, s));
    }
  }
}


void x3d_screen_draw_scanline_grad(int16 y, int16 left, int16 right, X3D_Color c, fp0x16 scale_left, fp0x16 scale_right, X3D_Color* color_tab, int16 z) {
  uint16 i;
  
  int16* z_buf = x3d_rendermanager_get()->zbuf;

  if(render_mode == 0) {
    scale_left = 0x7FFF;
    scale_right = 0x7FFF;
  }

  int16 map[3][3] = {
    { 1, 8, 4 },
    { 7, 6, 3 },
    { 5, 2, 9 }
  };
  
  uint16 total_c = 32;
  uint16 scale_bits = 5;

  if(right < left) return;
  
  int32 scale_slope = (((int32)scale_right - scale_left) << 8) / (right - left + 1);
  int32 scale = (int32)scale_left << 8;
  
  int32 err = 0;
  
  int32 mask = (1 << (16 - scale_bits)) - 1;
  int32 half = mask / 2;
  
  int16 zz = 0;
  
  if(z != 0) zz = 0x7FFF / z;
  
  for(i = left; i <= right; ++i) {
    if(scale < 0 && scale_slope < 0 && scale + scale_slope >= 0) x3d_assert(0);
    
    
    int32 val = (scale >> 8) + (render_mode >= 2 ? (int32)(scale >> 8) * map[i % 3][y % 3] / 10 : 0);
    
    int16 index = val >> (15 - scale_bits);
    
    if(index < 0) index = 0;
    
    if(index >= total_c)
      index = total_c - 1;

#if 1
    //if(zz > z_buf[y * window_surface->w + i]) {
      ((uint32 *)window_surface->pixels)[y * window_surface->w + i] = map_color_to_uint32(color_tab[index]);
      
      if(z > 0)
        z_buf[y * window_surface->w + i] = zz;
    //}
#endif
      
    scale += scale_slope;
  }
  
}

int32 recip_tab[1];

int32 fast_recip(int32* tab, uint16 val) {
  return (1L << 23) / val;
}

void x3d_screen_draw_scanline_texture_affine(X3D_Span* span, int16 y) {
  if(span->right.x < span->left.x) return;
  
  int32* tab = recip_tab;
  
  
  int32 next_z = fast_recip(tab, span->left.z >> 12);
  int32 next_u = ((span->left.u >> 5) * next_z);
  int32 next_v = ((span->left.v >> 5) * next_z);
  
  int32 prev_u, prev_v, prev_z;
  
  int16 dx = span->right.x - span->left.x;
  
  const int16 RUN = 32;
  
  int32 z_slope = x3d_val_slope2(span->right.z - span->left.z, dx);
  
  int32 uz_slope = x3d_val_slope2(span->right.u - span->left.u, dx) * RUN;
  int32 vz_slope = x3d_val_slope2(span->right.v - span->left.v, dx) * RUN;
  int32 zz_slope = z_slope * RUN;
  
  int32 uz = span->left.u;
  int32 vz = span->left.v;
  int32 zz = span->left.z;
  
  int32 i = span->left.x;
  
  X3D_Texture* tex = global_texture;
  int16* z_buf = x3d_rendermanager_get()->zbuf;
  uint32* pixels = window_surface->pixels;
  
  do {
    prev_u = next_u;
    prev_v = next_v;

    int32 z = zz;
    
    uz += uz_slope;
    vz += vz_slope;
    zz += zz_slope;
    
    int32 recip_z = fast_recip(tab, zz >> 12);
    next_u = ((uz >> 5) * recip_z);
    next_v = ((vz >> 5) * recip_z);
    
    int32 u_slope = x3d_val_slope2(next_u - prev_u, RUN);
    int32 v_slope = x3d_val_slope2(next_v - prev_v, RUN);
    
    
    int16 total = (span->right.x - i + 1 >= RUN ? RUN : span->right.x - i + 1);
    
    int32 u = prev_u;
    int32 v = prev_v;
    
    do {
      uint16 uu = (u >> 23) & (tex->w - 1);
      uint16 vv = (v >> 23) & (tex->w - 1);
      
      uint16 zz = z >> 15;
      
      if(zz >= z_buf[y * 640L + i]) {
        pixels[y * 640L + i] = map_color_to_uint32(tex->texel[(int32)vv * tex->w + uu]);
        z_buf[y * 640L + i] = zz;
      }
      
      u += u_slope;
      v += v_slope;
      z += z_slope;
      
      ++i;
      
    } while(--total > 0);
  } while(i <= span->right.x);
}

void x3d_screen_draw_scanline_texture(X3D_Span* span, int16 y) {
  x3d_screen_draw_scanline_texture_affine(span, y);
  return;
  
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
    
    //int16 uu = (x3d_fix_slope_val(&u) / zz);   //(((u / zz) >> 1) * 128) >> 15;
    //int16 vv = (x3d_fix_slope_val(&v) / zz);//(((v / zz) >> 1) * 128) >> 15;
    
    int16 recip = (1.0 / (z >> 15)) * 32767; //fast_recip(z >> 15);
    
    //x3d_log(X3D_INFO, "recip: %d", recip);
    
    
    int16 uu = (int64)x3d_fix_slope_val(&u) / zz;   //(((u / zz) >> 1) * 128) >> 15;
    int16 vv = (int64)x3d_fix_slope_val(&v) / zz;//(((v / zz) >> 1) * 128) >> 15;
    
    //x3d_log(X3D_INFO, "u: %d, v: %d", uu, vv);
    
    //uu /= 16;
    //vv /= 16;
    
    //int16 uu = (((u >> 16) * zz) * 192) >> 15;
    //int16 vv = (((v >> 16) * zz) * 192) >> 15;
    
    
    zz >>= 8;
    
    X3D_Color c = x3d_texture_get_texel(global_texture, uu, vv);
    
    if(zz >= z_buf[y * window_surface->w + i]) {
      ((uint32 *)window_surface->pixels)[y * window_surface->w + i] = map_color_to_uint32(c);
      z_buf[y * window_surface->w + i] = zz;
    }
    
    //u += u_slope;
    //v += v_slope;
    x3d_fix_slope_add(&u, &u_slope);
    x3d_fix_slope_add(&v, &v_slope);
    
    
    z += z_slope;
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
  
  if(!virtual_window) {
    //x3d_gray_dither(window_surface);
    SDL_Flip(window_surface);
  }
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
