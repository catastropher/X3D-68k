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

#include "OldRenderer.hpp"
#include "engine/EngineContext.hpp"
#include "error/Error.hpp"
#include "engine/Engine.hpp"
#include "system/File.hpp"
#include "Camera.hpp"
#include "util/StopWatch.hpp"
#include "level/Portal.hpp"
#include "entity/component/CameraComponent.hpp"
#include "entity/component/TransformComponent.hpp"
#include "entity/Entity.hpp"
#include "level/LevelManager.hpp"

static void x_renderer_init_console_vars(OldRenderer* renderer, Console* console)
{
    x_console_register_var(console, &renderer->fillColor, "render.fillColor", X_CONSOLEVAR_INT, "0", 0);
    x_console_register_var(console, &renderer->showFps, "render.showFps", X_CONSOLEVAR_BOOL, "0", 0);
    x_console_register_var(console, &renderer->frustumClip, "frustumClip", X_CONSOLEVAR_BOOL, "1", 0);
    x_console_register_var(console, &renderer->wireframe, "wireframe", X_CONSOLEVAR_BOOL, "0", 0);
    x_console_register_var(console, &renderer->maxFramesPerSecond, "maxFps", X_CONSOLEVAR_INT, "60", 0);
}

static void cmd_res(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: res [WxH] -> changes screen resolution\n");
        return;
    }

    Screen* screen = context->screen;
    
    if(screen->handlers.isValidResolution == NULL)
        x_system_error("No screen valid resolution checking createEntityCallback");
    
    int w, h;
    sscanf(argv[1], "%dx%d", &w, &h);
    
    if(!screen->handlers.isValidResolution(w, h))
    {
        x_console_print(context->console, "Invalid resolution for platform\n");
        return;
    }

    context->renderer->screenW = w;
    context->renderer->screenH = h;

    x_console_printf(context->console, "Effect will take place on next vidrestart\n");
}

static void cmd_fov(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: fov [angle] -> changes field of view\n");
        return;
    }
    
    fp fov = fp::fromFloat(atof(argv[1]) * 256.0 / 360.0);
    
    if(fov == 0)
    {
        x_console_print(context->console, "Invalid angle\n");
        return;
    }

    context->renderer->fov = fov;
    x_console_printf(context->console, "Effect will take place on next vidrestart\n");
}

static void cmd_vidrestart(EngineContext* context, int argc, char* argv[])
{
    //x_enginecontext_restart_video(context);
}

static void cmd_fullscreen(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: fullscreen [WxH] -> enables/disables fullscreen mode\n");
        return;
    }

    context->renderer->fullscreen = atoi(argv[1]);

    x_console_printf(context->console, "Effect will take place on next vidrestart\n");;
}

#include "error/Log.hpp"

// Prints the ID of the surface we're currently looking at
static void cmd_surfid(EngineContext* context, int argc, char* argv[])
{
    // Render a frame so we can get the span data
    //x_engine_render_frame(context);

    Screen* screen = context->screen;
    Vec2 center = screen->getCenter();

    int surfaceId = x_ae_context_find_surface_point_is_in(&context->renderer->activeEdgeContext, center.x, center.y,
                                                          context->levelManager->getCurrentLevel());
    
    if(surfaceId != -1)
    {
        x_console_printf(context->console, "Looking at surface #%d\n", surfaceId);
    }
    else
    {
        x_console_print(context->console, "Not looking at a surface\n");
    }
}

static void cmd_lighting(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: lighting [1/0] -> enables/disables lighting");
        return;
    }

    context->renderer->enableLighting = atoi(argv[1]);
    x_cache_flush(&context->renderer->surfaceCache);
}

static void cmd_scalescreen(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_printf(context->console, "Usage: %s [0/1] - enables/disables screen scaling", argv[0]);
        return;
    }
    
#ifndef __nspire__
    x_console_print(context->console, "Screen scaling only available on Nspire\n");
    return;
#endif
    
//    Camera* cam = context->getScreen()->cameraListHead;
//    int w;
//    int h;
//
//    context->getRenderer()->scaleScreen = atoi(argv[1]) != 0;
//
//    if(context->getRenderer()->scaleScreen)
//    {
//        w = 320 / 2;
//        h = 240 / 2;
//    }
//    else
//    {
//        w = 320;
//        h = 240;
//    }
//
//    // FIXME: don't hardcode angle
//    cam->viewport.init(Vec2(0, 0), w, h, fp(X_ANG_60));
}

#define MAX_SURFACES 300
#define MAX_EDGES 1000
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

static void x_renderer_init_colormap(OldRenderer* renderer, const X_Palette* palette)
{
    renderer->colorMap = (X_Color*)x_malloc(256 * X_COLORMAP_SHADES_PER_COLOR * sizeof(X_Color));
    
    const int TOTAL_FULLBRIGHTS = 32;
    
    for(int i = 0; i < 256 - TOTAL_FULLBRIGHTS; ++i)
        init_color_shade(renderer->colorMap, palette, i);
    
    for(int i = 256 - TOTAL_FULLBRIGHTS; i < 256; ++i)
    {
        for(int j = 0; j < X_COLORMAP_SHADES_PER_COLOR; ++j)
            renderer->colorMap[i * X_COLORMAP_SHADES_PER_COLOR + j] = i;
    }    
}

static void x_renderer_init_dynamic_lights(OldRenderer* renderer)
{
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        renderer->dynamicLights[i].flags = X_LIGHT_FREE;
        renderer->dynamicLights[i].id = i;
    }
    
    renderer->dynamicLightsNeedingUpdated = 0;
}

static void x_renderer_console_cmds(Console* console)
{
    x_console_register_cmd(console, "res", cmd_res);
    x_console_register_cmd(console, "fov", cmd_fov);
    x_console_register_cmd(console, "vidrestart", cmd_vidrestart);    
    x_console_register_cmd(console, "fullscreen", cmd_fullscreen);    
    x_console_register_cmd(console, "surfid", cmd_surfid);    
    x_console_register_cmd(console, "lighting", cmd_lighting);
    x_console_register_cmd(console, "scalescreen", cmd_scalescreen);
}

static void x_renderer_set_default_values(OldRenderer* renderer, Screen* screen, int fov)
{
    renderer->screenW = screen->getW();
    renderer->screenH = screen->getH();
    renderer->videoInitialized = 0;
    renderer->fullscreen = 0;
    renderer->fov = fov;
    renderer->enableLighting = 1;
    renderer->scaleScreen = 0;
    renderer->maxFramesPerSecond = 60;
}

OldRenderer::OldRenderer(Screen* screen, Console* console, int fov)
    : activeEdgeContext(8000, 8000, 30000, screen)
{
    x_renderer_console_cmds(console);
    x_renderer_init_console_vars(this, console);
    x_renderer_set_default_values(this, screen, fov);
    
    x_cache_init(&surfaceCache, 500000 * 4, "surfacecache");     // TODO: this size should be configurable
    x_renderer_init_colormap(this, screen->palette);
    x_renderer_init_dynamic_lights(this);
}

void x_renderer_cleanup(OldRenderer* renderer)
{
    x_cache_cleanup(&renderer->surfaceCache);
    
    x_free(renderer->colorMap);
}

void x_renderer_restart_video(OldRenderer* renderer, Screen* screen)
{
    // FIXME: need to reconstruct object
    
    //x_ae_context_cleanup(&renderer->activeEdgeContext);
    //x_ae_context_init(&renderer->activeEdgeContext, screen, MAX_ACTIVE_EDGES, MAX_EDGES, MAX_SURFACES);
}


