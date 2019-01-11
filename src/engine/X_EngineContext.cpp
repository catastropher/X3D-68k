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
#include "object/X_CameraObject.h"

#include <time.h>

#ifdef X_GET_TIME_USING_SDL
#include <SDL/SDL.h>
#endif

#include "X_EngineContext.h"
#include "error/X_error.h"
#include "render/X_RenderContext.h"
#include "system/Clock.hpp"

static inline void init_object_factory(X_EngineContext* context)
{
    x_objectfactory_init(context->getGameObjectFactory(), context);
    
    context->activeObjectHead.nextActive = &context->activeObjectTail;
    context->activeObjectHead.prevActive = NULL;
    
    context->activeObjectTail.nextActive = NULL;
    context->activeObjectTail.prevActive = &context->activeObjectHead;
    
    x_gameobject_register_default_types(context->getGameObjectFactory());
}

static inline void init_screen(X_EngineContext* context, X_Config* config)
{
    x_screen_init(context->getScreen(), config->screen->w, config->screen->h, &config->screen->screenHandlers);
    x_screen_set_palette(context->getScreen(), config->screen->palette);
}

static inline void init_main_font(X_EngineContext* context, const char* fontFileName, int fontW, int fontH)
{  
    if(!context->getMainFont()->loadFromFile(fontFileName))
        x_system_error("Failed to load font");
}

static inline void init_console(X_EngineContext* context)
{
    x_console_init(context->getConsole(), context, context->getMainFont());
    x_console_print(context->getConsole(), "Console initialized.\n");
}

static inline void init_keystate(X_EngineContext* context)
{
    x_keystate_init(context->getKeyState());
}

static inline void cleanup_object_factory(X_EngineContext* context)
{
    x_objectfactory_cleanup(context->getGameObjectFactory());
}

static inline void cleanup_screen(X_EngineContext* context)
{
    x_screen_cleanup(context->getScreen());
}


static inline void cleanup_console(X_EngineContext* context)
{
    x_console_cleanup(context->getConsole());
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
    init_main_font(context, config->font, 8, 8);     // TODO: this should be configurable
    init_console(context);
    init_keystate(context);
    
    x_mousestate_init(context->getMouseState(), context->getConsole(), context->getScreen());
    
    x_renderer_init(context->getRenderer(), context->getConsole(), context->getScreen(), config->screen->fov.toFp16x16());
}

void x_enginecontext_restart_video(X_EngineContext* context)
{
    int newW = context->getRenderer()->screenW;
    int newH = context->getRenderer()->screenH;
    x_fp16x16 newFov = context->getRenderer()->fov;
    
    if(context->getScreen()->handlers.restartVideo == NULL)
        x_system_error("No restart video callback");
    
    x_screen_restart_video(context->getScreen(), newW, newH, newFov);
    x_renderer_restart_video(context->getRenderer(), context->getScreen());
    
    context->getScreen()->handlers.restartVideo(context, context->getScreen()->handlers.userData);
    
    // FIXME: this is a hack
    if(context->getMouseState()->mouseLook)
        x_console_execute_cmd(context->getConsole(), "mouse.look 1");
}

////////////////////////////////////////////////////////////////////////////////
/// Cleans up an engine context.
////////////////////////////////////////////////////////////////////////////////
void x_enginecontext_cleanup(X_EngineContext* context)
{
    cleanup_object_factory(context);
    cleanup_screen(context);
    cleanup_console(context);
    x_bsplevel_cleanup(context->getCurrentLevel());
    x_renderer_cleanup(context->getRenderer());
    
    context->getScreen()->handlers.cleanupVideo(context, context->getScreen()->handlers.userData);
}

void x_enginecontext_update_time(X_EngineContext* context)
{
    context->lastFrameStart = context->frameStart;
    context->frameStart = Clock::getTicks();
}

X_Time x_enginecontext_get_time(const X_EngineContext* context)
{
    return context->frameStart;
}

void x_enginecontext_get_rendercontext_for_camera(X_EngineContext* engineContext, X_CameraObject* cam, X_RenderContext* dest)
{
    dest->cam = cam;
    dest->camPos = x_cameraobject_get_position(cam);
    dest->canvas = &engineContext->getScreen()->canvas;
    dest->zbuf = engineContext->getScreen()->zbuf;
    dest->currentFrame = x_enginecontext_get_frame(engineContext);
    dest->engineContext = engineContext;
    dest->level = engineContext->getCurrentLevel();
    dest->renderer = engineContext->getRenderer();
    dest->screen = engineContext->getScreen();
    dest->viewFrustum = &cam->viewport.viewFrustum;
    dest->viewMatrix = &cam->viewMatrix;
    dest->renderer = engineContext->getRenderer();
}

