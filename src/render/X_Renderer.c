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

#ifdef __nspire__
#include <libndls.h>
#endif

#include "X_Renderer.h"
#include "engine/X_EngineContext.h"
#include "error/X_error.h"
#include "engine/X_Engine.h"
#include "system/X_File.h"

static void x_renderer_init_console_vars(X_Renderer* renderer, X_Console* console)
{
    x_console_register_var(console, &renderer->fillColor, "render.fillColor", X_CONSOLEVAR_INT, "0", 0);
    x_console_register_var(console, &renderer->showFps, "render.showFps", X_CONSOLEVAR_BOOL, "0", 0);
    x_console_register_var(console, &renderer->frustumClip, "frustumClip", X_CONSOLEVAR_BOOL, "1", 0);
    x_console_register_var(console, &renderer->mipLevel, "mipLevel", X_CONSOLEVAR_INT, "0", 0);
    x_console_register_var(console, &renderer->renderMode, "rendermode", X_CONSOLEVAR_INT, "3", 0);
}

static void cmd_res(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage: res [WxH] -> changes screen resolution\n");
        return;
    }
    
    X_Screen* screen = &context->screen;
    
    if(screen->handlers.isValidResolution == NULL)
        x_system_error("No screen valid resolution checking callback");
    
    int w, h;
    sscanf(argv[1], "%dx%d", &w, &h);
    
    if(!screen->handlers.isValidResolution(w, h))
    {
        x_console_print(&context->console, "Invalid resolution for platform\n");
        return;
    }
    
    context->renderer.screenW = w;
    context->renderer.screenH = h;
    
    x_console_printf(&context->console, "Effect will take place on next vidrestart\n");
}

static void cmd_vidrestart(X_EngineContext* context, int argc, char* argv[])
{
    x_enginecontext_restart_video(context);
}

static void cmd_fullscreen(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage: fullscreen [WxH] -> enables/disables fullscreen mode\n");
        return;
    }
    
    context->renderer.fullscreen = atoi(argv[1]);
    
    x_console_printf(&context->console, "Effect will take place on next vidrestart\n");;
}

#include "error/X_log.h"

static void cmd_spanProfile(X_EngineContext* context, int argc, char* argv[])
{
    int count[640] = { 0 };
    
    for(int j = 0; j < context->renderer.activeEdgeContext.nextAvailableSurface - context->renderer.activeEdgeContext.surfacePool; ++j)
    {
        X_AE_Surface* surface = context->renderer.activeEdgeContext.surfacePool + j;
        
        for(int i = 0; i < surface->totalSpans; ++i)
        {
            X_AE_Span* span = surface->spans + i;
            ++count[span->x2 - span->x1];
        }
    }
    
    for(int i = 0; i < x_screen_w(&context->screen); ++i)
    {
        x_log("%d:   %d", i, count[i]);
    }
}    

// Prints the ID of the surface we're currently looking at
static void cmd_surfid(X_EngineContext* context, int argc, char* argv[])
{
    // Render a frame so we can get the span data
    //x_engine_render_frame(context);
    
    int centerX = x_screen_w(&context->screen) / 2;
    int centerY = x_screen_h(&context->screen) / 2;
    
    int surfaceId = x_ae_context_find_surface_point_is_in(&context->renderer.activeEdgeContext, centerX, centerY, &context->currentLevel);
    
    if(surfaceId != -1)
        x_console_printf(&context->console, "Looking at surface #%d\n", surfaceId);
    else
        x_console_print(&context->console, "Not looking at a surface\n");
}

static void cmd_lighting(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage: lighting [1/0] -> enables/disables lighting");
        return;
    }
    
    context->renderer.enableLighting = atoi(argv[1]);
    x_cache_flush(&context->renderer.surfaceCache);
}

#define MAX_SURFACES 1000
#define MAX_EDGES 5000
#define MAX_ACTIVE_EDGES 5000

static int convert_shade(int intensity, int shade)
{
    return (intensity * (shade) + 16) / 32;
}

static void init_color_shade(X_Color* colorMap, const X_Palette* palette, X_Color color)
{
    for(int i = 0; i < X_COLORMAP_SHADES_PER_COLOR; ++i)
    {
        unsigned char r, g, b;
        x_palette_get_rgb(palette, color, &r, &g, &b);
        
        int rr = X_MIN(255, convert_shade(r, i));
        int gg = X_MIN(255, convert_shade(g, i));
        int bb = X_MIN(255, convert_shade(b, i));
        
        colorMap[(int)color * X_COLORMAP_SHADES_PER_COLOR + i] = x_palette_get_closest_color_from_rgb(palette, rr, gg, bb);
    }
}

static void x_renderer_init_colormap(X_Renderer* renderer, const X_Palette* palette)
{
    renderer->colorMap = x_malloc(256 * X_COLORMAP_SHADES_PER_COLOR * sizeof(X_Color));
    
    const int TOTAL_FULLBRIGHTS = 32;
    
    for(int i = 0; i < 256 - TOTAL_FULLBRIGHTS; ++i)
        init_color_shade(renderer->colorMap, palette, i);
    
    for(int i = 256 - TOTAL_FULLBRIGHTS; i < 256; ++i)
    {
        for(int j = 0; j < X_COLORMAP_SHADES_PER_COLOR; ++j)
            renderer->colorMap[i * X_COLORMAP_SHADES_PER_COLOR + j] = i;
    }    
}

static void x_renderer_init_dynamic_lights(X_Renderer* renderer)
{
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        renderer->dynamicLights[i].flags = X_LIGHT_FREE;
        renderer->dynamicLights[i].id = i;
    }
    
    renderer->dynamicLightsNeedingUpdated = 0;
}

void x_renderer_init(X_Renderer* renderer, X_Console* console, X_Screen* screen, int fov)
{
    x_console_register_cmd(console, "res", cmd_res);    
    x_console_register_cmd(console, "vidrestart", cmd_vidrestart);    
    x_console_register_cmd(console, "fullscreen", cmd_fullscreen);    
    x_console_register_cmd(console, "surfid", cmd_surfid);    
    x_console_register_cmd(console, "lighting", cmd_lighting);
    
    static X_ConsoleCmd cmdSpanProfile = { "spanprofile", cmd_spanProfile };
    x_console_register_cmd(console, &cmdSpanProfile);
    
    x_renderer_init_console_vars(renderer, console);
    
    x_ae_context_init(&renderer->activeEdgeContext, screen, MAX_ACTIVE_EDGES, MAX_EDGES, MAX_SURFACES);
    x_cache_init(&renderer->surfaceCache, 3000000, "surfacecache");     // TODO: this size should be configurable
    
    renderer->screenW = x_screen_w(screen);
    renderer->screenH = x_screen_h(screen);
    renderer->videoInitialized = 0;
    renderer->fullscreen = 0;
    renderer->fov = fov;
    renderer->enableLighting = 1;
    
    renderer->usePalette = 0;
    
    x_renderer_init_colormap(renderer, screen->palette);
    x_renderer_init_dynamic_lights(renderer);
}

void x_renderer_restart_video(X_Renderer* renderer, X_Screen* screen)
{
    x_ae_context_cleanup(&renderer->activeEdgeContext);
    x_ae_context_init(&renderer->activeEdgeContext, screen, MAX_ACTIVE_EDGES, MAX_EDGES, MAX_SURFACES);
}

