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

#include "palette.h"
#include "Context.h"
#include "screen.h"
#include "keys.h"

_Bool init_sdl(Context* context, int screenW, int screenH)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    context->screen = SDL_SetVideoMode(screenW, screenH, 32, SDL_SWSURFACE);
    
    return context->screen != NULL;
}

void init_x3d(Context* context, int screenW, int screenH)
{
    X_EngineContext* xContext = &context->context;
    
    x_enginecontext_init(xContext, screenW, screenH);
    
    context->cam = x_cameraobject_new(xContext);
    x_viewport_init(&context->cam->viewport, (X_Vec2) { 0, 0 }, 640, 480, X_ANG_60);
    x_screen_attach_camera(&xContext->screen, context->cam);
    
    context->cam->base.orientation = x_quaternion_identity();
    
    X_Vec3_fp16x16 axis = x_vec3_make(0, X_FP16x16_ONE, 0);
    X_Quaternion rotation = x_quaternion_identity();
    
    //x_quaternion_init_from_axis_angle(&rotation, &axis, X_ANG_45);
    
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

void init(Context* context, int screenW, int screenH)
{
    context->quit = 0;
    
    init_sdl(context, screenW, screenH);
    init_x3d(context, screenW, screenH);
    build_color_table(context->screen);    
}

void cleanup_sdl(Context* context)
{
    SDL_Quit();
}

void cleanup_x3d(Context* context)
{
    x_enginecontext_cleanup(&context->context);
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
    handle_key_events(&context->context);
    
    _Bool adjustCam = 1;
    
    if(x_console_is_open(&context->context.console))
    {
        handle_console_keys(&context->context);
        return;
    }
    
    if(x_keystate_key_down(&context->context.keystate, X_KEY_OPEN_CONSOLE))
    {
        x_console_open(&context->context.console);
        x_keystate_reset_keys(&context->context.keystate);
        x_keystate_enable_text_input(&context->context.keystate);
        return;
    }

    if(key_is_down(SDLK_UP))
    {
        context->cam->angleX--;
        adjustCam = 1;
    }
    else if(key_is_down(SDLK_DOWN))
    {
        context->cam->angleX++;
        adjustCam = 1;
    }
    
    if(key_is_down(SDLK_LEFT))
    {
        context->cam->angleY++;
        adjustCam = 1;
    }
    else if(key_is_down(SDLK_RIGHT))
    {
        context->cam->angleY--;
        adjustCam = 1;
    }
    
    x_fp16x16 moveSpeed = 10 * 65536;
    X_Vec3 up, right, forward;
    
    x_mat4x4_extract_view_vectors(&context->cam->viewMatrix, &forward, &right, &up);
    
    if(key_is_down('w'))
    {
        context->cam->base.position = x_vec3_add_scaled(&context->cam->base.position, &forward, moveSpeed);
        adjustCam = 1;
    }
    else if(key_is_down('s'))
    {
        context->cam->base.position = x_vec3_add_scaled(&context->cam->base.position, &forward, -moveSpeed);
        adjustCam = 1;
    }
    
    if(key_is_down(SDLK_ESCAPE))
        context->quit = 1;
    
    if(adjustCam || 1)
    {
        x_cameraobject_update_view(context->cam);
    }
}

X_RenderContext* g_renderContext;
_Bool g_Pause;

void draw_grid(X_Vec3 center, int size, int step, X_RenderContext* rcontext, X_Color color)
{
    int totalSteps = size / step;
    
    for(int i = 0; i < totalSteps + 1; ++i)
    {
        int relStep = i - totalSteps / 2;
        X_Vec3 top = x_vec3_make(center.x + relStep * step, center.y, center.z - size / 2);
        X_Vec3 bottom = top;
        bottom.z += size;
        
        X_Ray3 ray = x_ray3_make(top, bottom);
        x_ray3d_render(&ray, rcontext, color);
    }
    
    for(int i = 0; i < totalSteps + 1; ++i)
    {
        int relStep = i - totalSteps / 2;
        X_Vec3 left = x_vec3_make(center.x - size / 2, center.y, center.z + relStep * step);
        X_Vec3 right = left;
        right.x += size;
        
        X_Ray3 ray = x_ray3_make(left, right);
        x_ray3d_render(&ray, rcontext, color);
    }
}

int main(int argc, char* argv[])
{
    Context context;
    
    init(&context, 640, 480);
    
    
    X_RenderContext rcontext;
    rcontext.cam = context.cam;
    rcontext.canvas = &context.context.screen.canvas;
    rcontext.viewFrustum = &rcontext.cam->viewport.viewFrustum;
    rcontext.viewMatrix = &context.cam->viewMatrix;
    
    x_screen_set_palette(&context.context.screen, x_palette_get_quake_palette());
    
    g_renderContext = &rcontext;
    
    context.cam->angleX = 0;
    context.cam->angleY = 0;
    context.cam->base.position = x_vec3_make(0, -200 * 65536, 0);
    
    x_cameraobject_update_view(context.cam);
    
    int frame = 0;

    X_ConsoleVar varHello;
    int hello;
    
    X_ConsoleVar varHeyThere;
    int heyThere;
   
    X_ConsoleVar varHeyza;
    int heyza;
    
    X_ConsoleVar varHen;
    int hen;
    
    X_ConsoleVar varHelpingHand;
    int helpingHand;

    X_BspLevel level;
    x_bsplevel_load_from_bsp_file(&level, "e1m1.bsp");
    
    x_console_register_var(&context.context.console, &varHello, &hello, "hello", X_CONSOLEVAR_INT, "50", 0);
    x_console_register_var(&context.context.console, &varHeyThere, &heyThere, "heyThere", X_CONSOLEVAR_INT, "50", 0);
    x_console_register_var(&context.context.console, &varHeyza, &heyza, "heyza", X_CONSOLEVAR_INT, "50", 0);
    x_console_register_var(&context.context.console, &varHen, &hen, "hen", X_CONSOLEVAR_INT, "50", 0);
    
    x_console_register_var(&context.context.console, &varHelpingHand, &helpingHand, "helpingHand", X_CONSOLEVAR_INT, "50", 0);
    
    while(!context.quit)
    {
        x_canvas_fill(&context.context.screen.canvas, 0);

        handle_keys(&context);
        
        //draw_grid(x_vec3_make(0, 0, 500), 32 * 16, 32, &rcontext, 4);
        
        x_bsplevel_render_wireframe(&level, &rcontext, context.context.screen.palette->brightRed);
        
        if(x_console_is_open(&context.context.console))
            x_console_render(&context.context.console);
        
        update_screen(&context);        
    }
    
    cleanup(&context);
}


