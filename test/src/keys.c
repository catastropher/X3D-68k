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

#include <SDL/SDL.h>
#include <X3D/X3D.h>

#include "Context.h"

#define TOTAL_SDL_KEYS 322
#define INVALID_KEY -1

#ifdef __nspire__

#define KEY_FORWARD '7'
#define KEY_BACKWARD '4'

#else

#define KEY_FORWARD 'w'
#define KEY_BACKWARD 's'

#endif

static _Bool sdlKeyState[TOTAL_SDL_KEYS];
static int x3dKeyMap[TOTAL_SDL_KEYS];

static void build_key_map(void)
{
    for(int i = 0; i < TOTAL_SDL_KEYS; ++i)
        x3dKeyMap[i] = INVALID_KEY;
    
    for(int i = 'a'; i <= 'z'; ++i)
        x3dKeyMap[i] = i;
    
    for(int i = 'A'; i <= 'Z'; ++i)
        x3dKeyMap[i] = i;
    
    for(int i = '0'; i <= '9'; ++i)
        x3dKeyMap[i] = i;
    
    x3dKeyMap[SDLK_SPACE] = ' ';
    
    x3dKeyMap[SDLK_LSHIFT] = x3dKeyMap[SDLK_RSHIFT] = X_KEY_SHIFT;
    x3dKeyMap[SDLK_RETURN] = '\n';
    x3dKeyMap[SDLK_BACKSPACE] = '\b';
    x3dKeyMap[SDLK_TAB] = '\t';
    
    const int AZERTY_SUPERSCRIPT_2 = 178;
    x3dKeyMap[SDLK_BACKQUOTE] = x3dKeyMap['~'] = x3dKeyMap[AZERTY_SUPERSCRIPT_2] = x3dKeyMap['9'] = X_KEY_OPEN_CONSOLE;
    
    x3dKeyMap[SDLK_UP] = X_KEY_UP;
    x3dKeyMap[SDLK_DOWN] = X_KEY_DOWN;
    x3dKeyMap[SDLK_LEFT] = X_KEY_LEFT;
    x3dKeyMap[X_KEY_RIGHT] = X_KEY_RIGHT;

    const char symbols[] = "!@#$%^&*()[]{}\\|:;'\",.<>/?-_=+";
    for(int i = 0; i < strlen(symbols); ++i)
        x3dKeyMap[(int)symbols[i]] = symbols[i];
}

void init_keys()
{
    SDL_EnableUNICODE(SDL_ENABLE);
    build_key_map();
}

static int convert_sdl_key_to_x3d_key(int sdlKey)
{
    if(sdlKey < 0 || sdlKey > TOTAL_SDL_KEYS)
        return INVALID_KEY;
    
    return x3dKeyMap[sdlKey];
}

void handle_key_events(X_EngineContext* context)
{
    SDL_Event ev;
    while(SDL_PollEvent(&ev))
    {
        if(ev.type == SDL_KEYDOWN)
        {
            sdlKeyState[ev.key.keysym.sym] = 1;
            
            int sdlKey;
            int unicodeSdlKey;
            
            int x3dKey;
            int unicodeX3dKey;
            
#ifndef __nspire__
            sdlKey = ev.key.keysym.sym;
            unicodeSdlKey = ev.key.keysym.unicode;
            
            if(!isprint(sdlKey))
                unicodeSdlKey = sdlKey;
            
            x3dKey = convert_sdl_key_to_x3d_key(sdlKey);
            unicodeX3dKey = convert_sdl_key_to_x3d_key(unicodeSdlKey);
#else
            sdlKey = ev.key.keysym.sym;
            unicodeSdlKey = sdlKey;
            x3dKey = convert_sdl_key_to_x3d_key(sdlKey);
            unicodeX3dKey = x3dKey;
#endif
            
            if(x3dKey != INVALID_KEY)
                x_keystate_send_key_press(&context->keystate, x3dKey, unicodeX3dKey);
        }
        else if(ev.type == SDL_KEYUP)
        {
            sdlKeyState[ev.key.keysym.sym] = 0;
            
            int sdlKey = ev.key.keysym.sym;
            int x3dKey = convert_sdl_key_to_x3d_key(sdlKey);
            
            if(x3dKey != INVALID_KEY)
                x_keystate_send_key_release(&context->keystate, x3dKey);
        }
    }
    
    x_keystate_handle_key_repeat(&context->keystate, x_enginecontext_get_time(context));
}

_Bool key_is_down(int sdlKey)
{
    return sdlKeyState[sdlKey];
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

_Bool everInLevel = 0;
_Bool onGround;

void attempt_move_cam(X_EngineContext* context, X_CameraObject* cam, X_Vec3_fp16x16 newPos)
{
    _Bool success = 0;
    
    X_Vec3 oldVelocity = cam->base.velocity;
    
    onGround = 0;
    
    for(int i = 0; i < 4 && !success; ++i)
    {
        if(!x_bsplevel_file_is_loaded(&context->currentLevel))
        {
            success = 1;
        }
        else
        {
            if(!everInLevel)
            {
                X_Vec3 point = x_vec3_fp16x16_to_vec3(&newPos);
                everInLevel = x_bsplevel_find_leaf_point_is_in(&context->currentLevel, &point)->contents != X_BSPLEAF_SOLID;
                success = 1;
            }
            else
            {
                X_RayTracer trace;
                x_raytracer_init(&trace, &context->currentLevel, &cam->base.position, &newPos, NULL);
                success = !x_raytracer_trace(&trace);
                
                if(!success)
                {
                    x_fp16x16 penDepth = -x_plane_point_distance_fp16x16(&trace.collisionPlane, &newPos);
                    printf("Pen depth: %f\n", x_fp16x16_to_float(penDepth));
                    newPos = x_vec3_add_scaled(&newPos, &trace.collisionPlane.normal, penDepth);
                    
                    X_Vec3* velocity = &cam->base.velocity;
                    
                    x_fp16x16 dot = x_vec3_fp16x16_dot(&trace.collisionPlane.normal, velocity);
                    *velocity = x_vec3_add_scaled(velocity, &trace.collisionPlane.normal, -dot);
                    
                    int type = 0;
                    
                    if(trace.collisionPlane.normal.y < x_fp16x16_from_float(0.7))
                        type = 1;
                    
                    if(trace.collisionPlane.normal.y == 0)
                        type = 2;
                    
                    if(type == 1 || type == 2)
                        onGround = 1;
                }
            }
        }
    }
    
    if(success)
    {
        cam->base.position = x_vec3_add(&cam->base.position, &cam->base.velocity);
        x_cameraobject_update_view(cam);
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
    
    if(key_is_down(' '))
    {
        if(onGround)
        {
            X_Vec3_fp16x16 launch = x_vec3_make(0, -x_fp16x16_from_int(6), 0);
            context->cam->base.velocity = x_vec3_add(&context->cam->base.velocity, &launch);
        }
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
    
    x_fp16x16 moveSpeed = 65536;
    X_Vec3 up, right, forward;
    
    x_fp16x16 gravityStrength = x_fp16x16_from_float(.25);
    X_Vec3_fp16x16 gravity = x_vec3_make(0, gravityStrength, 0);
    
    context->cam->base.velocity = x_vec3_add(&gravity, &context->cam->base.velocity);
    
    x_mat4x4_extract_view_vectors(&context->cam->viewMatrix, &forward, &right, &up);
    
    if(key_is_down(KEY_FORWARD))
    {
        context->cam->base.velocity = x_vec3_add_scaled(&context->cam->base.velocity, &forward, moveSpeed);
    }
    else if(key_is_down(KEY_BACKWARD))
    {
        context->cam->base.velocity = x_vec3_add_scaled(&context->cam->base.velocity, &forward, -moveSpeed);
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
    
    X_Vec3_fp16x16 newPos = x_vec3_add(&context->cam->base.position, &context->cam->base.velocity);
    attempt_move_cam(context->engineContext, context->cam, newPos);
    
    //if(adjustCam)
    //{
    //    x_cameraobject_update_view(context->cam);
   // }
}

