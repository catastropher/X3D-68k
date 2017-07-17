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

#include "X_Engine.h"
#include "error/X_error.h"
#include "system/X_File.h"
#include "error/X_log.h"
#include "render/X_RenderContext.h"


static _Bool g_engineInitialized = 0;
static X_EngineContext g_engineContext;

static X_EngineContext* x_engine_get_context(void)
{
    return &g_engineContext;
}

X_EngineContext* x_engine_init(int screenW, int screenH, const char* programPath)
{
    if(g_engineInitialized)
        x_system_error("Called x_engine_int() after engine already initialized");
    
    x_memory_init();
    x_filesystem_init(programPath);
    x_log_init();
    x_filesystem_add_search_path("../assets");
    
    X_EngineContext* engineContext = x_engine_get_context();
    x_enginecontext_init(engineContext, screenW, screenH);
    
    return engineContext;
}

void x_engine_cleanup(void)
{
    x_enginecontext_cleanup(x_engine_get_context());
    x_filesystem_cleanup();
    x_memory_free_all();
    x_log_cleanup();
}

static void x_engine_begin_frame(X_EngineContext* context)
{
    ++context->frameCount;
    x_enginecontext_update_time(context);
}

static void x_engine_draw_fps(X_EngineContext* context)
{
    int diff = context->frameStart - context->lastFrameStart;
    int fps;
    
    if(diff == 0)
        fps = 1000;
    else
        fps = 1000 / diff;
    
    char fpsStr[20];
    sprintf(fpsStr, "%d", fps);
    
    X_Vec2 pos = x_vec2_make(x_screen_w(&context->screen) - x_font_str_width(&context->mainFont, fpsStr), 0);
    x_canvas_draw_str(&context->screen.canvas, fpsStr, &context->mainFont, pos);
}

void add_ae_test_polygon(X_AE_Context* context);

void x_engine_render_frame(X_EngineContext* engineContext)
{
    x_engine_begin_frame(engineContext);
    
    if(engineContext->renderer.fillColor != X_RENDERER_FILL_DISABLED)
       x_canvas_fill(&engineContext->screen.canvas, engineContext->renderer.fillColor);
    
    for(X_CameraObject* cam = engineContext->screen.cameraListHead; cam != NULL; cam = cam->nextInCameraList)
    {
        X_RenderContext renderContext;
        x_enginecontext_get_rendercontext_for_camera(engineContext, cam, &renderContext);
        x_ae_context_reset(&engineContext->renderer.activeEdgeContext, &renderContext);
        
        x_cameraobject_render(cam, &renderContext);
       
        //add_ae_test_polygon(&engineContext->renderer.activeEdgeContext);
        
        x_ae_context_scan_edges(&engineContext->renderer.activeEdgeContext);
    }
    
    if(engineContext->renderer.showFps)
        x_engine_draw_fps(engineContext);
    
    if(x_console_is_open(&engineContext->console))
        x_console_render(&engineContext->console);
}

