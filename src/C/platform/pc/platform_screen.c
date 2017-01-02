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

#ifdef X3D_USE_SDL1
    #include <SDL/SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "X3D_common.h"
#include "X3D_init.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"
#include "X3D_trig.h"
#include "render/X3D_texture.h"
#include "render/X3D_palette.h"

static SDL_Surface* window_surface;

#ifndef X3D_USE_SDL1
static SDL_Window* window;
#endif


static int16 screen_w;
static int16 screen_h;
static int16 screen_scale;
static _Bool record;
static int16 record_frame;
static char record_name[1024];
static _Bool virtual_window;

static X3D_Color color_palette[256];

X3D_Texture panel_tex;
X3D_Texture brick_tex;
X3D_Texture floor_panel_tex;
X3D_Texture cube_tex;
X3D_Texture aperture_tex;

X3D_Texture* global_texture = &brick_tex;

void x3d_screen_zbuf_clear(void) {
    memset(x3d_rendermanager_get()->zbuf, 0x7F, screen_w * screen_h * 2);
}

void x3d_zbuf_clear(int16* zbuf, int16 w, int16 h) {
    memset(zbuf, 0x7F, (int32)w * h * sizeof(int16));
}

void x3d_platform_screen_build_color_palette(uint8 color_data[256][3]) {
    uint16 i;
    
    for(i = 0; i < 256; ++i) {
        color_palette[i] = x3d_rgb_to_color(color_data[i][0], color_data[i][1], color_data[i][2]);
    }
}

void x3d_set_texture(int16 id) {
  if(id == 0)       global_texture = &panel_tex;
  else if(id == 1)  global_texture = &brick_tex;
  else if(id == 2)  global_texture = &floor_panel_tex;
  else if(id == 3)  global_texture = &cube_tex;
  else if(id == 4)  global_texture = &aperture_tex;
}

void init_screen_manager(X3D_InitSettings* init) {
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
    x3d_state->screen_manager.scale_y = x3d_state->screen_manager.scale_x;
    
    x3d_state->screen_manager.buf = malloc(sizeof(X3D_ColorIndex) * screen_w * screen_h);
}

X3D_Color x3d_platform_screen_colorindex_to_color(X3D_ColorIndex index) {
    return color_palette[index];
}

_Bool init_sdl_virtual_screen(X3D_InitSettings* init) {
    uint32 rmask, gmask, bmask, amask;
    
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     *      on the endianness (byte order) of the machine */
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
    
    return X3D_TRUE;
}

#ifdef X3D_USE_SDL1

void init_sdl_window(X3D_InitSettings* init) {
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

#else

_Bool init_sdl_window(X3D_InitSettings* init) {
    window = SDL_CreateWindow("X3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, init->screen_w, init->screen_h, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    
    if(!window) {
      x3d_log(X3D_ERROR, "Failed to create SDL2 window");
      return X3D_FALSE;
    }
    
    window_surface = SDL_GetWindowSurface(window);
    
    x3d_log(X3D_INFO, "Window created");
    
    return X3D_TRUE;
}

#endif




X3D_INTERNAL _Bool x3d_platform_screen_init(X3D_InitSettings* init) {
  x3d_log(X3D_INFO, "SDL init");
  
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    x3d_log(X3D_ERROR, "Failed to init SDL: %s", SDL_GetError());
    return X3D_FALSE;
  }
  
  init_screen_manager(init);
  
  x3d_log(X3D_INFO, "Create window (w=%d, h=%d, pix_scale=%d, render_scale=%d)",
          init->screen_w, init->screen_h, init->screen_scale, x3d_state->screen_manager.scale_x);
  
  if(!(init->flags & X3D_INIT_VIRTUAL_SCREEN)) {
      init_sdl_window(init);
  }
  else {
      init_sdl_virtual_screen(init);
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

X3D_INTERNAL void x3d_platform_screen_cleanup(void) {
#ifdef X3D_USE_SDL1
    SDL_FreeSurface(window_surface);
#endif
    SDL_Quit();
}


void x3d_screen_zbuf_visualize(void) {
    uint32 i, j;
    
    for(i = 0; i < screen_h; ++i) {
        for(j = 0; j < screen_w; ++j) {
            int32 mx = 3000;
            int32 val = (mx - x3d_rendermanager_get()->zbuf[i * screen_w + j]) * 255 / mx;
            
            x3d_screen_draw_pix(j, i, x3d_rgb_to_color(val, val, val));
        }
    }
}

void x3d_screen_begin_record(const char* name) {
    strcpy(record_name, name);
    record = X3D_TRUE;
    record_frame = 0;
}

void x3d_screen_record_end(void) {
    record = X3D_FALSE;
}

#define BPP 32

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
  
  X3D_ScreenManager* screenman = x3d_screenmanager_get();
  
  if(!virtual_window) {
    //x3d_gray_dither(window_surface);
      
      int32 i;
      for(i = 0; i < (int32)screen_w * screen_h; ++i) {
          ((uint32 *)window_surface->pixels)[i] = map_color_to_uint32(color_palette[screenman->buf[i]]);
      }
      
#ifdef X3D_USE_SDL1
      SDL_Flip(window_surface);
#else
      
      SDL_UpdateWindowSurface(window);
#endif
  }
}

void* x3d_screen_get_internal(void) {
  return window_surface;
}

void x3d_screen_clear(X3D_Color color) {
  //SDL_FillRect(window_surface, NULL, map_color_to_uint32(color));
    
    X3D_ScreenManager* screenman = x3d_screenmanager_get();
    
    memset(screenman->buf, 0, sizeof(X3D_ColorIndex) * screen_w * screen_h);
}

void x3d_screen_draw_pix(int16 x, int16 y, X3D_ColorIndex color) {
  //uint32 c = map_color_to_uint32(color);
  
  if(x < 0 || x >= screen_w || y < 0 || y >= screen_h)
    return;
  
  for(int32 i = 0; i < screen_scale; ++i) {
    for(int32 d = 0; d < screen_scale; ++d) {
      int32 xx = x * screen_scale + d;
      int32 yy = y * screen_scale + i;
      
      //((uint32 *)window_surface->pixels)[yy * window_surface->w + xx] = c;
      
      x3d_screenmanager_get()->buf[yy * window_surface->w + xx] = color;
      
      //x3d_rendermanager_get()->zbuf[yy * screen_w + xx] = 0x7FFF;
    }
  }
}

void x3d_screen_set_internal_value(int16 x, int16 y, uint32 val) {
    if(x < 0 || x >= screen_w || y < 0 || y >= screen_h)
        return;
    
    ((uint32 *)window_surface->pixels)[y * window_surface->w + x] = val;
}

uint32 x3d_screen_get_internal_value(int16 x, int16 y) {
    if(x < 0 || x >= screen_w || y < 0 || y >= screen_h)
        return 0;
    
    return ((uint32 *)window_surface->pixels)[y * window_surface->w + x];
}

#define plot(_x, _y) x3d_screen_draw_pix(_x, _y, c)

void x3d_screen_draw_circle(int16 x0, int16 y0, int16 radius, X3D_Color c)
{
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;
 
    plot(x0, y0 + radius);
    plot(x0, y0 - radius);
    plot(x0 + radius, y0);
    plot(x0 - radius, y0);
 
    while(x < y) 
    {
        if(f >= 0) 
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;    
        plot(x0 + x, y0 + y);
        plot(x0 - x, y0 + y);
        plot(x0 + x, y0 - y);
        plot(x0 - x, y0 - y);
        plot(x0 + y, y0 + x);
        plot(x0 - y, y0 + x);
        plot(x0 + y, y0 - x);
        plot(x0 - y, y0 - x);
    }
}

void x3d_screen_draw_line(int16 x0, int16 y0, int16 x1, int16 y1, X3D_ColorIndex color) {
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

_Bool x3d_platform_screen_load_texture(X3D_Texture* tex, const char* file) {
    x3d_log(X3D_INFO, "File: %s", file);
    SDL_Surface* s = SDL_LoadBMP(file);
    
    if(!s) {
        x3d_log(X3D_ERROR, "Failed to load texture %s: %s", file, SDL_GetError());
        return X3D_FALSE;
    }
    
    x3d_texture_init(tex, s->w, s->h, 0);
    
    uint16 i, d;
    for(i = 0; i < s->h; ++i) {
        for(d = 0; d < s->w; ++d) {
            uint32 pix = getpixel(s, d, i);
            uint8 r, g, b;
            
            SDL_GetRGB(pix, s->format, &r, &g, &b);
            
            x3d_texture_set_texel(tex, d, i, x3d_color_to_colorindex(x3d_rgb_to_color(r, g, b)));
        }
    }
    
    SDL_FreeSurface(s);
    
    return X3D_TRUE;
}





