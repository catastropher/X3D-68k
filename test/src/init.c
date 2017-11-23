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

#include <X3D/X3D.h>

#include "init.h"
#include "screen.h"
#include "keys.h"

static void cam_screen_size_changed_callback(X_CameraObject* cam, X_Screen* screen, int fov)
{
    x_viewport_init(&cam->viewport, x_vec2_make(0, 0), x_screen_w(screen), x_screen_h(screen), fov);
}

static void init_camera(Context* context)
{
    X_EngineContext* engineContext = context->engineContext;
    
    context->cam = x_cameraobject_new(engineContext);
    x_viewport_init(&context->cam->viewport, (X_Vec2) { 0, 0 }, x_screen_w(&engineContext->screen), x_screen_h(&engineContext->screen), X_ANG_60);
    x_screen_attach_camera(&engineContext->screen, context->cam);
    context->cam->screenResizeCallback = cam_screen_size_changed_callback;
    
    context->cam->angleX = 0;
    context->cam->angleY = 0;
    context->cam->collider.position = x_vec3_make(0, -50 * 65536, -800 * 65536);
    context->cam->collider.velocity = x_vec3_origin();
    
    x_cameraobject_update_view(context->cam);
}

static void init_x3d(Context* context, int screenW, int screenH, const char* programPath)
{
    X_Config config;
    
    x_config_init(&config);
    x_config_set_program_path(&config, programPath);
    x_config_set_screen_defaults(&config, screenW, screenH, X_ANG_60, 0);
    screen_set_callbacks(context, &config);
    x_config_screen_set_palette(&config, x_palette_get_quake_palette());
    
    context->engineContext = x_engine_init(&config);
}

static X_Vec2 determine_default_platform_resolution(void)
{
#ifdef __nspire__
    return x_vec2_make(320, 240);
#else
    return x_vec2_make(640, 480);
#endif
}

void init(Context* context, const char* programPath)
{
    X_Vec2 resolution = determine_default_platform_resolution();
    init_x3d(context, resolution.x, resolution.y, programPath);    
    init_camera(context);    
    init_keys(context);
    
    x_console_execute_cmd(&context->engineContext->console, "searchpath ..");
    x_console_execute_cmd(&context->engineContext->console, "exec engine.cfg");
    
    screen_init_console_vars(&context->engineContext->console);
    
    context->quit = 0;
}

static void cleanup_sdl(Context* context)
{
    // Make sure we don't quit without putting the screen back to normal
    SDL_SetVideoMode(context->nativeResolutionW, context->nativeResolutionH, 32, SDL_SWSURFACE);
    
    SDL_Quit();
}

static void cleanup_x3d(Context* context)
{
    x_engine_cleanup();
}

void cleanup(Context* context)
{
    cleanup_keys(context);
    cleanup_sdl(context);
    cleanup_x3d(context);
}

