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

#include "palette.h"
#include "Context.h"

Uint32 sdl_getpixel(SDL_Surface *surface, int x, int y)
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

void sdl_putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
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

#ifdef __nspire__

static void update_screen_nspire(Context* context)
{
    unsigned short sdlColors[256];
    
    for(int i = 0; i < 256; ++i)
        sdlColors[i] = get_sdl_color_from_x_color(i);
    
    X_Color* pixel = context->context->screen.canvas.tex.texels;
    X_Color* pixelEnd = pixel + x_screen_w(&context->context->screen) * x_screen_h(&context->context->screen);
    
    unsigned short* pixelDest = context->screen->pixels;
    
    do
    {
        *pixelDest++ = sdlColors[*pixel++];
    } while(pixel < pixelEnd);
}

#endif

void update_screen(Context* context)
{
#ifdef __nspire__
    update_screen_nspire(context);
#else
    
    for(int i = 0; i < context->screen->h; ++i)
    {
        for(int j = 0; j < context->screen->w; ++j)
        {
            X_Color xColor = x_texture_get_texel(&context->context->screen.canvas.tex, j, i);
            unsigned int sdlColor = get_sdl_color_from_x_color(xColor);
            sdl_putpixel(context->screen, j, i, sdlColor);
        }
    }
#endif
    
    SDL_Flip(context->screen);
}

