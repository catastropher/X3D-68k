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

// Converts a bitmap image to an xtex

#include <X3D/X3D.h>
#include <SDL/SDL.h>

Uint32 sdl_getpixel(const SDL_Surface *surface, int x, int y)
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

void convert_sdlsurface_to_texture(const SDL_Surface* surface, const X_Palette* colorPalette, X_Texture* dest)
{
    x_texture_init(dest, surface->w, surface->h);
    
    for(int i = 0; i < surface->h; ++i)
    {
        for(int j = 0; j < surface->w; ++j)
        {
            Uint32 pixel = sdl_getpixel(surface, j, i);
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            
            X_Color x3dColor = x_palette_get_closest_color_from_rgb(colorPalette, r, g, b);
            
            x_texture_set_texel(dest, j, i, x3dColor);
        }
    }
}

void print_usage(const char* programName)
{
    printf("Usage: %s input.bmp output.xtex\n\n", programName);
    printf("Converts bitmaps into X3D's texture file format\n");
    printf("Note: this tool currently uses the quake color palette\n");
}

_Bool convert_texture(const char* bmpSource, const char* xtexDest)
{
    SDL_Surface* bitmap = SDL_LoadBMP(bmpSource);
    if(bitmap == NULL)
    {
        fprintf(stderr, "Failed to load BMP: %s\n", SDL_GetError());
        return 0;
    }
    
    const X_Palette* palette = x_palette_get_quake_palette();
    X_Texture tex;
    convert_sdlsurface_to_texture(bitmap, palette, &tex);
    
    x_texture_save_to_xtex_file(&tex, xtexDest);
    
    return 1;
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        print_usage(argv[0]);
        return -1;
    }
    
    return (convert_texture(argv[1], argv[2]) ? 0 : -1);
}

