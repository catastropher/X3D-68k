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

#include "Context.h"
#include "keys.h"

#ifdef __nspire__
static unsigned short oldColorPalette[256];
#endif

#ifdef __nspire__

// void scale_screen(Context* context)
// {
//     unsigned char* texels = context->engineContext->screen.canvas.texels;
    
//     unsigned short* screen = (unsigned short*)REAL_SCREEN_BASE_ADDRESS;
    
//     for(int i = 0; i < 240 / 2; ++i)
//     {
//         for(int j = 0; j < 320 / 2; ++j)
//         {
//             unsigned int tex = texels[i * 320 + j];
//             unsigned int res = tex | (tex << 8);
            
//             screen[(i * 2) * (320 / 2) + j] = res;
//             screen[(i * 2 + 1) * (320 / 2) + j] = res;
//         }
//     }
// }

// extern "C"
// {

// void copy_screen(unsigned char* dest, unsigned char* src);

// }

// static void update_screen_nspire(Context* context)
// {
//     //if(context->engineContext->renderer.scaleScreen)
//     //    scale_screen(context);
//     //else
    
//     copy_screen((unsigned char*)REAL_SCREEN_BASE_ADDRESS, context->engineContext->screen.canvas.texels);
    
//     //memcpy(REAL_SCREEN_BASE_ADDRESS, context->engineContext->screen.canvas.texels, 320 * 240);
// }

// static unsigned short map_rgb_to_nspire_color(const unsigned char color[3])
// {
//     unsigned short r = color[0] / 8;
//     unsigned short g = color[1] / 8;
//     unsigned short b = color[2] / 8;
    
//     x_assert(r <= 31 && g <= 31 && b <= 31, "Bad index");
    
//     return b | (g << 5) | (r << 10);
// }

// static unsigned short* get_hardware_palette_registers(void)
// {
//     return (unsigned short*)0xC0000200;
// }

// static void save_old_palette(void)
// {
//     unsigned short* paletteColor = get_hardware_palette_registers();
//     for(int i = 0; i < 256; ++i)
//         oldColorPalette[i] = paletteColor[i];
// }

// static void restore_old_palette(void)
// {
//     unsigned short* paletteColor = get_hardware_palette_registers();
//     for(int i = 0; i < 256; ++i)
//         paletteColor[i] = oldColorPalette[i];
// }

// static void set_palette(const X_Palette* palette)
// {
//     unsigned int* paletteColor = (unsigned int*)get_hardware_palette_registers();
//     for(int i = 0; i < 256; i += 2)
//     {
//         unsigned int lo = map_rgb_to_nspire_color(&palette->colorRGB[i][0]);
//         unsigned int hi = map_rgb_to_nspire_color(&palette->colorRGB[i + 1][0]);
        
//         *paletteColor++ = (lo | (hi << 16));
//     }
// }

// static unsigned int color_to_4bit(const X_Palette* palette, X_Color color)
// {
//     return palette->grayscaleTable[color];
// }

// static unsigned int pack_color(const X_Palette* palette, X_Color* color)
// {
//     return (color_to_4bit(palette, color[7]) << 28) +
//     (color_to_4bit(palette, color[6]) << 24) +
//     (color_to_4bit(palette, color[5]) << 20) +
//     (color_to_4bit(palette, color[4]) << 16) +
//     (color_to_4bit(palette, color[3]) << 12) +
//     (color_to_4bit(palette, color[2]) << 8) +
//     (color_to_4bit(palette, color[1]) << 4) +
//     (color_to_4bit(palette, color[0]) << 0);
// }

// static void pack_screen(const X_Palette* palette, X_Color* screen)
// {
//     unsigned int* lcd = (unsigned int*)REAL_SCREEN_BASE_ADDRESS;
    
//     for(int i = 0; i < 320 * 240; i += 8)
//         *lcd++ = pack_color(palette, screen + i);
// }

#endif

// static bool record = 0;
// static char recordBaseFile[256];
// static int recordFrame;
// static int frameId;

// void screen_update(Context* context)
// {
// #ifdef __nspire__
//     int type = lcd_type();
    
//     if(type != SCR_320x240_4)
//         update_screen_nspire(context);
//     else
//         pack_screen(context->engineContext->screen.palette, context->engineContext->screen.canvas.texels);
// #else
    
//     x_texture_to_sdl_surface(&context->engineContext->getScreen()->canvas, context->engineContext->getScreen()->palette, context->screen);
    
//     if(record && (x_enginecontext_get_frame(context->engineContext) % recordFrame) == 0) {
//         char fileName[512];
//         sprintf(fileName, "%s%.4d.bmp", recordBaseFile, frameId);
        
//         SDL_SaveBMP(context->screen, fileName);
        
//         ++frameId;
        
//         SDL_Delay(50);
//     }
    
//     SDL_Flip(context->screen);
// #endif
// }

// static void cmd_record(X_EngineContext* context, int argc, char* argv[])
// {
//     if(argc != 3)
//     {
//         x_console_printf(context->getConsole(), "Usage: record [frameBaseFileName] [nth frame] -> records every nth frame to bitmap files\n");
//         return;
//     }
    
//     strcpy(recordBaseFile, argv[1]);
//     recordFrame = X_MAX(1, atoi(argv[2]));
//     record = 1;
//     frameId = 0;
    
//     x_console_printf(context->getConsole(), "Recording enabled every %d frames", recordFrame);
// }

// static void cmd_endrecord(X_EngineContext* context, int argc, char* argv[])
// {
//     x_console_printf(context->getConsole(), "Recording stopped\n");
//     record = 0;
// }

void screen_init_console_vars(Console* console)
{
    // x_console_register_cmd(console, "record", cmd_record);    
    // x_console_register_cmd(console, "endrecord", cmd_endrecord);
}

static bool is_valid_resolution_callback(int w, int h)
{
#ifdef __nspire__
    return w == 320 && h == 240;
#endif
    
    return 1;
}



static void video_restart_callback(X_EngineContext* engineContext, void* userData)
{
//     Context* context = (Context*)userData;
//     X_Screen* screen = engineContext->getScreen();
    
//     if(!engineContext->getRenderer()->videoInitialized)
//     {
//         if(SDL_Init(SDL_INIT_VIDEO) != 0)
//             x_system_error("Failed to initialize SDL");
        
//         // Grab the native desktop resolution
//         const SDL_VideoInfo* info = SDL_GetVideoInfo();
        
//         context->nativeResolutionW = info->current_w;
//         context->nativeResolutionH = info->current_h;
        
// #ifdef __nspire__
//         int type = lcd_type();
        
//         if(type != SCR_320x240_4)
//         {
//             lcd_init(SCR_320x240_8);
//             memset(REAL_SCREEN_BASE_ADDRESS, 0, 320 * 240);     // Prevent the screen from flashing when we switch palettes
//             save_old_palette();
//             set_palette(engineContext->screen.palette);
//         }
// #endif
//     }

// #ifndef __nspire__
//     int flags = SDL_SWSURFACE;
    
//     if(engineContext->getRenderer()->fullscreen)
//         flags |= SDL_FULLSCREEN;
    
//     context->screen = SDL_SetVideoMode(x_screen_w(screen), x_screen_h(screen), 32, flags);
    
//     if(!context->screen)
//         x_system_error("Failed to set video mode");
// #endif
}

static void cleanup_video_callback(X_EngineContext* context, void* userData)
{
#ifdef __nspire__
    //lcd_init(SCR_TYPE_INVALID);
    //restore_old_palette();
#endif
}

void screen_set_callbacks(Context* context, ScreenConfig& config)
{
    config
        .isValidResolutionCallback(is_valid_resolution_callback)
        .restartVideoCallback(video_restart_callback)
        .cleanupVideoCallback(cleanup_video_callback)
        .userData(context);
}

