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

#pragma once

#include <SDL/SDL.h>

#include "render/X_Texture.h"
#include "render/X_Palette.h"
#include "error/X_error.h"

static inline Uint32 x_sdl_getpixel(SDL_Surface *surface, int x, int y)
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

static inline void x_sdl_putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
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

static inline void x_texture_to_sdl_surface(const X_Texture* src, const X_Palette* palette, SDL_Surface* dest)
{
    x_assert(dest->w >= src->getW() && dest->h >= src->getH(), "SDL surface dimensions too small");
    
    unsigned int colorTable[256];
    
    for(int i = 0; i < 256; ++i)
    {
        unsigned char r, g, b;
        x_palette_get_rgb(palette, i, &r, &g, &b);
        colorTable[i] = SDL_MapRGB(dest->format, r, g, b);
    }
    
    for(int i = 0; i < src->getH(); ++i)
    {
        for(int j = 0; j < src->getW(); ++j)
        {
            x_sdl_putpixel(dest, j, i, colorTable[src->getTexel({ j, i })]);
        }
    }
}



