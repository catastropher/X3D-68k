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

#include "X_config.h"

#include <time.h>

#ifdef X_GET_TIME_USING_SDL
#include <SDL/SDL.h>
#endif

#include "X_EngineContext.h"
#include "error/X_error.h"
#include "render/X_RenderContext.h"

static inline void init_object_factory(X_EngineContext* context)
{
    x_factory_init(&context->gameObjectFactory, 5, 10);
}

static inline void init_screen(X_EngineContext* context, X_Config* config)
{
    x_screen_init(&context->screen, config->screenW, config->screenH, &config->screenHandlers);
    x_screen_set_palette(&context->screen, config->palette);
}

static inline void init_main_font(X_EngineContext* context, const char* fontFileName, int fontW, int fontH)
{
    _Bool fontLoaded = x_font_load_from_xtex_file(&context->mainFont, fontFileName, fontW, fontH);
    
    if(!fontLoaded)
        x_system_error("Failed to load font");
}

static inline void init_console(X_EngineContext* context)
{
    x_console_init(&context->console, context, &context->mainFont);
    x_console_print(&context->console, "Console initialized.\n");
}

static inline void init_keystate(X_EngineContext* context)
{
    x_keystate_init(&context->keystate);
}

static inline void init_level(X_EngineContext* context)
{
    x_bsplevel_init_empty(&context->currentLevel);
}



static inline void cleanup_object_factory(X_EngineContext* context)
{
    x_factory_cleanup(&context->gameObjectFactory);
}

static inline void cleanup_screen(X_EngineContext* context)
{
    x_screen_cleanup(&context->screen);
}

static inline void cleanup_main_font(X_EngineContext* context)
{
    x_font_cleanup(&context->mainFont);
}

static inline void cleanup_console(X_EngineContext* context)
{
    x_console_cleanup(&context->console);
}

////////////////////////////////////////////////////////////////////////////////
/// Initializes an engine context.
///
/// @param context  - context to initialize
/// @param screenW  - width of the screen
/// @param screenH  - height of the screen
///
/// @note Make sure to call @ref x_enginecontext_cleanup() when done to release
///     the allocated resources!
////////////////////////////////////////////////////////////////////////////////
void x_enginecontext_init(X_EngineContext* context, X_Config* config)
{
    context->frameCount = 1;
    x_enginecontext_update_time(context);
    context->lastFrameStart = context->frameStart;
    
    init_object_factory(context);
    init_screen(context, config);
    init_main_font(context, "font.xtex", 8, 8);     // TODO: this should be configurable
    init_console(context);
    init_keystate(context);
    
    x_mousestate_init(&context->mouseState, &context->console, &context->screen);
    
    init_level(context);
    
    x_renderer_init(&context->renderer, &context->console, &context->screen, config->fov);
}

void x_enginecontext_restart_video(X_EngineContext* context)
{
    int newW = context->renderer.screenW;
    int newH = context->renderer.screenH;
    x_fp16x16 newFov = context->renderer.fov;
    
    if(context->screen.handlers.restartVideo == NULL)
        x_system_error("No restart video callback");
    
    x_screen_restart_video(&context->screen, newW, newH, newFov);
    x_renderer_restart_video(&context->renderer, &context->screen);
    
    context->screen.handlers.restartVideo(context, context->screen.handlers.userData);
    
    // FIXME: this is a hack
    if(context->mouseState.mouseLook)
        x_console_execute_cmd(&context->console, "mouse.look 1");
}

////////////////////////////////////////////////////////////////////////////////
/// Cleans up an engine context.
////////////////////////////////////////////////////////////////////////////////
void x_enginecontext_cleanup(X_EngineContext* context)
{
    cleanup_object_factory(context);
    cleanup_screen(context);
    cleanup_main_font(context);
    cleanup_console(context);
    x_bsplevel_cleanup(&context->currentLevel);
    x_renderer_cleanup(&context->renderer);
    
    context->screen.handlers.cleanupVideo(context, context->screen.handlers.userData);
}

void x_enginecontext_update_time(X_EngineContext* context)
{
    context->lastFrameStart = context->frameStart;
    
#ifndef X_GET_TIME_USING_SDL
    context->frameStart = clock() * 1000 / CLOCKS_PER_SEC;
#else
    context->frameStart = SDL_GetTicks();
#endif
}

X_Time x_enginecontext_get_time(const X_EngineContext* context)
{
    return context->frameStart;
}

void x_enginecontext_get_rendercontext_for_camera(X_EngineContext* engineContext, X_CameraObject* cam, X_RenderContext* dest)
{
    dest->cam = cam;
    X_Vec3_fp16x16 camPos = x_cameraobject_get_position(cam);
    dest->camPos = x_vec3_fp16x16_to_vec3(&camPos);
    dest->canvas = &engineContext->screen.canvas;
    dest->zbuf = engineContext->screen.zbuf;
    dest->currentFrame = x_enginecontext_get_frame(engineContext);
    dest->engineContext = engineContext;
    dest->level = &engineContext->currentLevel;
    dest->renderer = &engineContext->renderer;
    dest->screen = &engineContext->screen;
    dest->viewFrustum = &cam->viewport.viewFrustum;
    dest->viewMatrix = &cam->viewMatrix;
    dest->renderer = &engineContext->renderer;
}

