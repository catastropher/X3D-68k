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

#include "Context.h"

static void draw_fps(X_EngineContext* context)
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
    x_texture_draw_str(&context->screen.canvas, fpsStr, &context->mainFont, pos);
}

static void draw_crosshair(X_EngineContext* engineContext)
{
    X_Color white = engineContext->screen.palette->white;
    X_Texture* tex = &engineContext->screen.canvas;
    
    int centerX = tex->w / 2;
    int centerY = tex->h / 2;
    
    x_texture_set_texel(tex, centerX - 1, centerY, white);
    x_texture_set_texel(tex, centerX + 1, centerY, white);
    x_texture_set_texel(tex, centerX, centerY - 1, white);
    x_texture_set_texel(tex, centerX, centerY + 1, white);
}

X_Light* add_light(X_Renderer* renderer)
{
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        if(x_light_is_free(renderer->dynamicLights + i))
        {
            renderer->dynamicLights[i].flags &= ~X_LIGHT_FREE;
            return renderer->dynamicLights + i;
        }
    }
    
    return NULL;
}


static void update_dynamic_lights(X_EngineContext* engineContext)
{
    _Bool down = x_keystate_key_down(&engineContext->keystate, 'q');
    static _Bool lastDown;
    X_CameraObject* cam = engineContext->screen.cameraListHead;
    
    if(down && !lastDown)
    {
        X_Vec3_fp16x16 up, right, forward;
        x_mat4x4_extract_view_vectors(&cam->viewMatrix, &forward, &right, &up);
        
        X_Light* light = add_light(&engineContext->renderer);
        
        if(light == NULL)
            return;
        
        light->position = cam->base.position;
        light->intensity = 300;
        light->direction = forward;
        light->flags |= X_LIGHT_ENABLED;
    }
    
    X_Light* lights = engineContext->renderer.dynamicLights;
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        if(x_light_is_enabled(engineContext->renderer.dynamicLights + i))
        {
            lights[i].position = x_vec3_add_scaled(&lights[i].position, &lights[i].direction, 10 << 16);
        }
    }
    
    lastDown = down;
}

static void draw_hud(X_EngineContext* engineContext)
{
    draw_crosshair(engineContext);
    draw_fps(engineContext);
}

void render(Context* context)
{
    X_EngineContext* engineContext = context->engineContext;
    
    update_dynamic_lights(engineContext);
    x_engine_render_frame(engineContext);
    draw_hud(engineContext);
    
    if(x_console_is_open(&engineContext->console))
        x_console_render(&engineContext->console);
}

