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
}

void init(Context* context, int screenW, int screenH)
{
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

int main(int argc, char* argv[])
{
    Context context;
    
    init(&context, 640, 480);
    
    X_Vec2 a = { 10, 10 };
    X_Vec2 b = { 200, 100 };
    
    X_Cube cube;
    x_cube_init(&cube, 50, 50, 50);
    
    X_Cube rotatedCube;
    
    int invSqrt3 = (1.0 / sqrt(3)) * 65536;
    
    X_File file;
    x_file_open_reading(&file, "test");
    x_file_close(&file);
    
    for(int i = 0; i < 256; ++i)
    {
        x_canvas_fill(&context.context.screen.canvas, 0);
        
        X_Mat4x4 rotation;
        X_Quaternion quat;
        
        X_Vec3_fp16x16 axis = x_vec3_make(invSqrt3, invSqrt3, invSqrt3);
        x_quaternion_init_from_axis_angle(&quat, &axis, i);
        x_quaternion_to_mat4x4(&quat, &rotation);
        
        x_cube_transform(&cube, &rotatedCube, &rotation);
        
        x_cube_translate(&rotatedCube, x_vec3_make(0, 0, 500));
        x_cube_render(&rotatedCube, context.cam, &context.context.screen.canvas, 4);
        
        update_screen(&context);
    }
    
    sleep(3);
    cleanup(&context);
}


