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
#include "keys.h"

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
    unsigned int sdlColors[256];
    
    for(int i = 0; i < 256; ++i)
        sdlColors[i] = get_sdl_color_from_x_color(i);
    
    X_Color* pixel = context->context->screen.canvas.tex.texels;
    X_Color* pixelEnd = pixel + x_screen_w(&context->context->screen) * x_screen_h(&context->context->screen);
    
    unsigned int* pixelDest = REAL_SCREEN_BASE_ADDRESS;
    
    do
    {
        *pixelDest++ = (sdlColors[pixel[0]]) + (sdlColors[pixel[1]] << 16);
        pixel += 2;
    } while(pixel < pixelEnd);
}

#endif

static _Bool record = 0;
static char recordBaseFile[256];
static int recordFrame;
static int frameId;

float g_zbuf[480][640];

void update_screen(Context* context)
{
#ifdef __nspire__
    if(!context->context->renderer.usePalette)
        update_screen_nspire(context);
    else
    {
        memcpy(REAL_SCREEN_BASE_ADDRESS, context->context->screen.canvas.tex.texels, 320 * 240);
        //lcd_blit(context->context->screen.canvas.tex.texels, SCR_320x240_8);
        return;
    }
#else
    
    x_texture_to_sdl_surface(&context->context->screen.canvas.tex, context->context->screen.palette, context->screen);
    
    if(record && (x_enginecontext_get_frame(context->context) % recordFrame) == 0) {
        char fileName[512];
        sprintf(fileName, "%s%.4d.bmp", recordBaseFile, frameId);
        
        SDL_SaveBMP(context->screen, fileName);
        
        ++frameId;
        
        SDL_Delay(50);
    }
    
    SDL_Flip(context->screen);
#endif
}

static void cmd_record(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 3)
    {
        x_console_printf(&context->console, "Usage: record [frameBaseFileName] [nth frame] -> records every nth frame to bitmap files\n");
        return;
    }
    
    strcpy(recordBaseFile, argv[1]);
    recordFrame = X_MAX(1, atoi(argv[2]));
    record = 1;
    frameId = 0;
    
    x_console_printf(&context->console, "Recording enabled every %d frames", recordFrame);
}

static void cmd_endrecord(X_EngineContext* context, int argc, char* argv[])
{
    x_console_printf(&context->console, "Recording stopped\n");
    record = 0;
}

void screen_init_console_vars(X_Console* console)
{
    static X_ConsoleCmd cmdRecord = { "record", cmd_record };
    x_console_register_cmd(console, &cmdRecord);
    
    static X_ConsoleCmd endRecord = { "endrecord", cmd_endrecord };
    x_console_register_cmd(console, &endRecord);
}


