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
    
    x_canvas_fill(&context.context.screen.canvas, 0);
    x_canvas_draw_line(&context.context.screen.canvas, a, b, 5);
    
    update_screen(&context);
    
    sleep(3);
    cleanup(&context);
}


