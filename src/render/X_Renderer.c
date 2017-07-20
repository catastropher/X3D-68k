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

static void x_renderer_init_console_vars(X_Renderer* renderer, X_Console* console)
{
    x_console_register_var(console, &renderer->varFillColor, &renderer->fillColor, "render.fillColor", X_CONSOLEVAR_INT, "0", 0);
    x_console_register_var(console, &renderer->varShowFps, &renderer->showFps, "render.showFps", X_CONSOLEVAR_BOOL, "0", 0);
}

static void cmd_enablePalette(X_EngineContext* context, int argc, char* argv[])
{
    if(context->renderer.usePalette)
    {
        x_console_print(&context->console, "Already switched to palette\n");
        return;
    }
    
#ifdef __nspire__
    context->renderer.usePalette = 1;
    lcd_init(SCR_320x240_8);
#else
    x_console_print(&context->console, "Not available on PC\n");
#endif
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

#define MAX_SURFACES 1000
#define MAX_EDGES 5000
#define MAX_ACTIVE_EDGES 5000

void x_renderer_init(X_Renderer* renderer, X_Console* console, X_Screen* screen, int fov)
{
    static X_ConsoleCmd cmdEnablePalette = { "enablePalette", cmd_enablePalette };
    x_console_register_cmd(console, &cmdEnablePalette);
    
    static X_ConsoleCmd cmdRes = { "res", cmd_res };
    x_console_register_cmd(console, &cmdRes);
    
    static X_ConsoleCmd cmdVidrestart = { "vidrestart", cmd_vidrestart };
    x_console_register_cmd(console, &cmdVidrestart);
    
    static X_ConsoleCmd cmdFullscreen = { "fullscreen", cmd_fullscreen };
    x_console_register_cmd(console, &cmdFullscreen);
    
    x_renderer_init_console_vars(renderer, console);
    x_ae_context_init(&renderer->activeEdgeContext, screen, MAX_ACTIVE_EDGES, MAX_EDGES, MAX_SURFACES);
    
    renderer->screenW = x_screen_w(screen);
    renderer->screenH = x_screen_h(screen);
    renderer->videoInitialized = 0;
    renderer->fullscreen = 0;
    renderer->fov = fov;
    
    renderer->usePalette = 0;
}

void x_renderer_restart_video(X_Renderer* renderer, X_Screen* screen)
{
    x_ae_context_cleanup(&renderer->activeEdgeContext);
    x_ae_context_init(&renderer->activeEdgeContext, screen, MAX_ACTIVE_EDGES, MAX_EDGES, MAX_SURFACES);
}

