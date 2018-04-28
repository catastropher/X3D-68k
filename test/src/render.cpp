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
    
    X_Vec2 pos = x_vec2_make(x_screen_w(&context->screen) - context->mainFont.calcWidthOfStr(fpsStr), 0);
    context->screen.canvas.drawStr(fpsStr, context->mainFont, pos);
}

static void draw_crosshair(X_EngineContext* engineContext)
{
    X_Color white = engineContext->screen.palette->white;
    X_Texture* tex = &engineContext->screen.canvas;
    
    int centerX = tex->getW() / 2;
    int centerY = tex->getH() / 2;
    
    tex->setTexel({ centerX - 1, centerY }, white);
    tex->setTexel({ centerX + 1, centerY }, white);
    tex->setTexel({ centerX, centerY - 1 }, white);
    tex->setTexel({ centerX, centerY + 1 }, white);
}

X_Light* add_light(X_Renderer* renderer)
{
    for(int i = 0; i < X_RENDERER_MAX_LIGHTS; ++i)
    {
        if(x_light_is_free(renderer->dynamicLights + i))
        {
            //renderer->dynamicLights[i].flags &= ~X_LIGHT_FREE;
            return renderer->dynamicLights + i;
        }
    }
    
    return NULL;
}


static void update_dynamic_lights(X_EngineContext* engineContext)
{
    bool down = x_keystate_key_down(&engineContext->keystate, (X_Key)'q');
    static bool lastDown;
    X_CameraObject* cam = engineContext->screen.cameraListHead;
    
    if(down && !lastDown)
    {
        Vec3 up, right, forward;
        x_mat4x4_extract_view_vectors(&cam->viewMatrix, &forward, &right, &up);
        
        X_Light* light = add_light(&engineContext->renderer);
        
        if(light == NULL)
            return;
        
        light->position = cam->base.position;
        light->intensity = 300;
        light->direction = forward;
        //light->flags |= X_LIGHT_ENABLED;
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

