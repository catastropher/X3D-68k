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
#include <SDL/SDL.h>
#include <unistd.h>
#include <math.h>

#include "Context.h"
#include "screen.h"
#include "keys.h"

#ifdef __nspire__

#define KEY_FORWARD '7'
#define KEY_BACKWARD '4'

#else

#define KEY_FORWARD 'w'
#define KEY_BACKWARD 's'

#endif

void cam_screen_size_changed_callback(X_CameraObject* cam, X_Screen* screen, int fov)
{
    x_viewport_init(&cam->viewport, x_vec2_make(0, 0), x_screen_w(screen), x_screen_h(screen), fov);
}

void init_x3d(Context* context, int screenW, int screenH, const char* programPath)
{
    X_Config config;
    x_config_init(&config);
    
    x_config_set_program_path(&config, programPath);
    
    x_config_set_screen_defaults(&config, screenW, screenH, X_ANG_60, 0);
    screen_set_callbacks(context, &config);
    x_config_screen_set_palette(&config, x_palette_get_quake_palette());
    
    X_EngineContext* xContext = context->engineContext = x_engine_init(&config);
    
    context->cam = x_cameraobject_new(xContext);
    x_viewport_init(&context->cam->viewport, (X_Vec2) { 0, 0 }, screenW, screenH, X_ANG_60);
    x_screen_attach_camera(&xContext->screen, context->cam);
    context->cam->screenResizeCallback = cam_screen_size_changed_callback;
    
    context->cam->base.orientation = x_quaternion_identity();
    
    X_Quaternion rotation = x_quaternion_identity();
    
    X_Quaternion newOrientation;
    x_quaternion_mul(&context->cam->base.orientation, &rotation, &newOrientation);
    context->cam->base.orientation = newOrientation;
    
    X_Vec3 up, right, forward;
    x_gameobject_extract_view_vectors(&context->cam->base, &forward, &right, &up);
    
    x_vec3_print(&up, "Up");
    x_vec3_print(&forward, "Foreward");
    x_vec3_print(&right, "Right");
    
    X_Vec3 camPos = x_vec3_make(0, 0, 0);
    
    x_viewport_update_frustum(&context->cam->viewport, &camPos, &forward, &right, &up);
    x_frustum_print(&context->cam->viewport.viewFrustum);
}

void init(Context* context, int screenW, int screenH, const char* programPath)
{
    context->quit = 0;
    
    init_x3d(context, screenW, screenH, programPath);
    init_keys();
    
    x_console_execute_cmd(&context->engineContext->console, "searchpath ..");
    x_console_execute_cmd(&context->engineContext->console, "exec engine.cfg");
}

void cleanup_sdl(Context* context)
{
    // Make sure we don't quit without putting the screen back to normal
    SDL_SetVideoMode(context->nativeResolutionW, context->nativeResolutionH, 32, SDL_SWSURFACE);
    
    SDL_Quit();
}

void cleanup_x3d(Context* context)
{
    x_engine_cleanup();
}

void cleanup(Context* context)
{
    cleanup_sdl(context);
    cleanup_x3d(context);
}

void handle_console_keys(X_EngineContext* context)
{
    X_Key key;
    while(x_keystate_dequeue(&context->keystate, &key))
    {
        if(key == X_KEY_OPEN_CONSOLE)
        {
            x_console_close(&context->console);
            x_keystate_reset_keys(&context->keystate);
            x_keystate_disable_text_input(&context->keystate);
            return;
        }
        
        x_console_send_key(&context->console, key);
    }
}

void handle_keys(Context* context)
{
    handle_key_events(context->engineContext);
    
    _Bool adjustCam = 0;
    
    if(key_is_down(SDLK_ESCAPE))
        context->quit = 1;
    
    if(x_console_is_open(&context->engineContext->console))
    {
        handle_console_keys(context->engineContext);
        return;
    }
    
    if(x_keystate_key_down(&context->engineContext->keystate, X_KEY_OPEN_CONSOLE))
    {
        x_console_open(&context->engineContext->console);
        x_keystate_reset_keys(&context->engineContext->keystate);
        x_keystate_enable_text_input(&context->engineContext->keystate);
        return;
    }

    if(key_is_down(SDLK_UP))
    {
        context->cam->angleX -= 2;
        adjustCam = 1;
    }
    else if(key_is_down(SDLK_DOWN))
    {
        context->cam->angleX += 2;
        adjustCam = 1;
    }
    
    if(key_is_down(SDLK_LEFT))
    {
        context->cam->angleY += 2;
        adjustCam = 1;
    }
    else if(key_is_down(SDLK_RIGHT))
    {
        context->cam->angleY -= 2;
        adjustCam = 1;
    }
    
    x_fp16x16 moveSpeed = 10 * 65536;
    X_Vec3 up, right, forward;
    
    x_mat4x4_extract_view_vectors(&context->cam->viewMatrix, &forward, &right, &up);
    
    if(key_is_down(KEY_FORWARD))
    {
        context->cam->base.position = x_vec3_add_scaled(&context->cam->base.position, &forward, moveSpeed);
        adjustCam = 1;
    }
    else if(key_is_down(KEY_BACKWARD))
    {
        context->cam->base.position = x_vec3_add_scaled(&context->cam->base.position, &forward, -moveSpeed);
        adjustCam = 1;
    }
    
    if(key_is_down('d'))
    {
        context->cam->base.position = x_vec3_add_scaled(&context->cam->base.position, &right, moveSpeed);
        adjustCam = 1;
    }
    else if(key_is_down('a'))
    {
        context->cam->base.position = x_vec3_add_scaled(&context->cam->base.position, &right, -moveSpeed);
        adjustCam = 1;
    }
    
    if(adjustCam)
    {
        x_cameraobject_update_view(context->cam);
    }
}

void screen_init_console_vars(X_Console* console);

void gameloop(Context* context)
{
    while(!context->quit)
    {
        x_engine_render_frame(context->engineContext);
    
        handle_keys(context);
        
        screen_update(context);
    }
}

int main(int argc, char* argv[])
{
    Context context;
    
    int w, h;

#ifdef __nspire__
    w = 320;
    h = 240;
#else
    w = 640;
    h = 480;
#endif
    
    init(&context, w, h, argv[0]);
    screen_init_console_vars(&context.engineContext->console);
    
    context.cam->angleX = 0;
    context.cam->angleY = 0;
    context.cam->base.position = x_vec3_make(0, -50 * 65536, -800 * 65536);
    
    x_cameraobject_update_view(context.cam);
    
    gameloop(&context);
    
    cleanup(&context);
}


