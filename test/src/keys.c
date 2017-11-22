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

static Context* g_Context;

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
    x3dKeyMap[SDLK_RIGHT] = X_KEY_RIGHT;

    const char symbols[] = "!@#$%^&*()[]{}\\|:;'\",.<>/?-_=+";
    for(int i = 0; i < strlen(symbols); ++i)
        x3dKeyMap[(int)symbols[i]] = symbols[i];
}

static void cmd_demo(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage demo [filename] -> begins recording a demo\n");
        return;
    }
    
    if(x_demorecorder_is_recording(&g_Context->demoRecorder))
    {
        x_console_print(&context->console, "A demo is already recording\n");
        return;
    }
    
    if(!x_demorecorder_record(&g_Context->demoRecorder, argv[1]))
        x_console_print(&context->console, "Failed to start recording demo");
    
    x_keystate_reset_keys(&context->keystate);
    x_console_force_close(&context->console);
}

static void cmd_playdemo(X_EngineContext* context, int argc, char* argv[])
{
    if(argc < 2)
    {
        x_console_print(&context->console, "Usage playdemo [filename] [opt base record filename] -> plays a demo\n");
        return;
    }
    
    if(x_demorecorder_is_recording(&g_Context->demoRecorder))
    {
        x_console_print(&context->console, "A demo is being recorded\n");
        return;
    }
    
    if(!x_demoplayer_play(&g_Context->demoPlayer, argv[1]))
        x_console_print(&context->console, "Failed to start playing demo");
    
    x_keystate_reset_keys(&context->keystate);
    x_console_force_close(&context->console);
    
    if(argc != 3)
        return;
    
    char recordCmd[X_FILENAME_MAX_LENGTH];
    sprintf(recordCmd, "record %s 4", argv[2]);
    
    x_console_execute_cmd(&context->console, recordCmd);
}

void init_keys(Context* context)
{
    g_Context = context;
    
    SDL_EnableUNICODE(SDL_ENABLE);
    build_key_map();
    
    x_demorecorder_init(&context->demoRecorder, context->cam, &context->engineContext->keystate);
    x_demoplayer_init(&context->demoPlayer, context->cam, &context->engineContext->keystate);
    
    x_console_register_cmd(&context->engineContext->console, "demo", cmd_demo);
    x_console_register_cmd(&context->engineContext->console, "playdemo", cmd_playdemo);
}

void cleanup_keys(Context* context)
{
    x_demorecorder_cleanup(&context->demoRecorder);
    x_demoplayer_cleanup(&context->demoPlayer);
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
_Bool hitVerticalWall;

_Bool attempt_move_cam(X_EngineContext* context, X_CameraObject* cam, X_Vec3_fp16x16 newPos)
{
    _Bool success = 0;
    
    X_Vec3 oldVelocity = cam->base.velocity;
    
    onGround = 0;
    hitVerticalWall = 0;
    
    X_RayTracer trace;
    
    _Bool somethingHit = 0;
    _Bool attemptSnap = cam->base.velocity.y > 0;
    
    int i;
    for(i = 0; i < 4 && !success; ++i)
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
                x_raytracer_init(&trace, &context->currentLevel, &cam->base.position, &newPos, NULL);
                success = !x_raytracer_trace(&trace);
                
                if(!success)
                {
                    somethingHit = 1;
                    newPos = trace.collisionPoint;
                    
//                     x_fp16x16 penDepth = -x_plane_point_distance_fp16x16(&trace.collisionPlane, &newPos);
//                     //printf("Pen depth: %f\n", x_fp16x16_to_float(penDepth));
//                     
//                     newPos = x_vec3_add_scaled(&newPos, &trace.collisionPlane.normal, penDepth);
//                     
                    X_Vec3* velocity = &cam->base.velocity;
                    
                    x_fp16x16 dot = x_vec3_fp16x16_dot(&trace.collisionPlane.normal, velocity);
                    *velocity = x_vec3_add_scaled(velocity, &trace.collisionPlane.normal, -dot);
                    
                    newPos = x_vec3_add(&newPos, velocity);
                    
                    //if(X_SIGNOF(velocity->z) != X_SIGNOF(oldVelocity.y))
                    //    velocity->y = 0;
                    
                    
                    int type = 0;
                    
                    if(trace.collisionPlane.normal.y < x_fp16x16_from_float(0.7))
                        type = 1;
                    
                    if(trace.collisionPlane.normal.y == 0)
                    {
                        type = 2;
                    }
                    
                    if(!onGround && (type == 1 || type == 2))
                    {
//                         printf("----------\n");
//                         printf("Iter = %d\n", i);
//                         x_vec3_fp16x16_print(&oldVelocity, "Old velocity");
//                         x_vec3_fp16x16_print(velocity, "New velocity");
//                         x_vec3_fp16x16_print(&trace.collisionPlane.normal, "Normal");
//                         printf("Dot product: %f\n", x_fp16x16_to_float(dot));
//                         printf("----------\n");
                    }
                    
                    if(type == 1)
                        onGround = 1;
                    
                    if(type == 2)
                        hitVerticalWall = 1;
                }
            }
        }
    }
    
    if(somethingHit)
    {
        x_fp16x16 speed = x_vec3_fp16x16_length(&cam->base.velocity);
        
        if(speed != 0)
        {
        
            const x_fp16x16 friction = x_fp16x16_from_float(4.0);
            x_fp16x16 newSpeed = speed - friction;
            if(newSpeed < 0)
                newSpeed = 0;
            
            x_fp16x16 maxSpeed = x_fp16x16_from_float(50);
            if(newSpeed > maxSpeed)
                newSpeed = maxSpeed;
            
            newSpeed = x_fp16x16_div(newSpeed, speed);
            
            X_Vec3 origin = x_vec3_origin();
            cam->base.velocity = x_vec3_add_scaled(&origin, &cam->base.velocity, newSpeed);
        }
    }

    if(success)
    {
        // Snap to the floor
        if(everInLevel)
        {
            X_Vec3_fp16x16 beneath = newPos;
            const x_fp16x16 SNAP_TOLERANCE = x_fp16x16_from_float(3);
            beneath.y += SNAP_TOLERANCE;
            
            x_raytracer_init(&trace, &context->currentLevel, &newPos, &beneath, NULL);
            
            if(x_raytracer_trace(&trace))
            {
                newPos = trace.collisionPoint;
                onGround = 1;
            }
        }
        
        cam->base.position = newPos;//x_vec3_add(&cam->base.position, &cam->base.velocity);
        x_cameraobject_update_view(cam);
        
    }
    
    return success;
}

void handle_keys(Context* context)
{
    handle_key_events(context->engineContext);
    
    _Bool wasPlaying = 0;
    
    if(x_demoplayer_is_playing(&g_Context->demoPlayer))
    {
        x_demoplayer_play_frame(&g_Context->demoPlayer);
        wasPlaying = 1;
    }
    else if(x_demorecorder_is_recording(&g_Context->demoRecorder))
    {
        x_demorecorder_save_frame(&g_Context->demoRecorder);
    }
    
    if(wasPlaying && !x_demoplayer_is_playing(&g_Context->demoPlayer))
    {
        x_console_execute_cmd(&context->engineContext->console, "endrecord");
    }
    
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
    
    X_KeyState* keyState = &context->engineContext->keystate;
    
    if(x_keystate_key_down(keyState, ' '))
    {
        if(onGround)
        {
            X_Vec3_fp16x16 launch = x_vec3_make(0, -x_fp16x16_from_int(6), 0);
            context->cam->base.velocity = x_vec3_add(&context->cam->base.velocity, &launch);
        }
    }

    if(x_keystate_key_down(keyState, X_KEY_UP))
    {
        context->cam->angleX -= 2;
        adjustCam = 1;
    }
    else if(x_keystate_key_down(keyState, X_KEY_DOWN))
    {
        context->cam->angleX += 2;
        adjustCam = 1;
    }
    
    if(x_keystate_key_down(keyState, X_KEY_LEFT))
    {
        context->cam->angleY += 2;
        adjustCam = 1;
    }
    else if(x_keystate_key_down(keyState, X_KEY_RIGHT))
    {
        context->cam->angleY -= 2;
        adjustCam = 1;
    }
    
    x_fp16x16 moveSpeed = 65536 * 3;
    X_Vec3 up, right, forward;
    
    x_fp16x16 gravityStrength = x_fp16x16_from_float(.25);
    X_Vec3_fp16x16 gravity = x_vec3_make(0, gravityStrength, 0);
    
//     if(everInLevel)
//         context->cam->base.velocity = x_vec3_add(&gravity, &context->cam->base.velocity);
    
    x_mat4x4_extract_view_vectors(&context->cam->viewMatrix, &forward, &right, &up);
    
    if(everInLevel && !key_is_down(SDLK_LCTRL))
        forward.y = 0;
    
    context->cam->base.position = x_cameraobject_get_position(context->cam);
        
    if(onGround || !everInLevel || key_is_down(SDLK_LCTRL))
    {
        if(x_keystate_key_down(keyState, KEY_FORWARD))
        {
            context->cam->base.velocity = x_vec3_add_scaled(&context->cam->base.velocity, &forward, moveSpeed);
        }
        else if(x_keystate_key_down(keyState, KEY_BACKWARD))
        {
            context->cam->base.velocity = x_vec3_add_scaled(&context->cam->base.velocity, &forward, -moveSpeed);
        }
        
        if(x_keystate_key_down(keyState, 'd'))
        {
            context->cam->base.velocity = x_vec3_add_scaled(&context->cam->base.velocity, &right, moveSpeed);
            adjustCam = 1;
        }
        else if(x_keystate_key_down(keyState, 'a'))
        {
            context->cam->base.velocity = x_vec3_add_scaled(&context->cam->base.velocity, &right, -moveSpeed);
            adjustCam = 1;
        }
    }
   
    if(!everInLevel || key_is_down(SDLK_LCTRL))
    {
        X_Vec3_fp16x16 newPos = x_vec3_add(&context->cam->base.position, &context->cam->base.velocity);
        X_Vec3 point = x_vec3_fp16x16_to_vec3(&newPos);
        everInLevel = x_engine_level_is_loaded(context->engineContext)
            && x_bsplevel_find_leaf_point_is_in(&context->engineContext->currentLevel, &point)->contents != X_BSPLEAF_SOLID;
            
        context->cam->base.position = newPos;
        x_cameraobject_update_view(context->cam);
        
        context->cam->base.velocity = x_vec3_origin();
    }
    else
    {
        context->cam->collider.velocity = context->cam->base.velocity;
        x_boxcollider_update(&context->cam->collider, &context->engineContext->currentLevel);
        
        context->cam->base.velocity = context->cam->collider.velocity;
        
        onGround = context->cam->collider.flags & X_BOXCOLLIDER_ON_GROUND;
        
        x_cameraobject_update_view(context->cam);
    }
   
//     X_Vec3_fp16x16 oldVelocity = context->cam->base.velocity;
//     X_Vec3_fp16x16 oldPos = context->cam->base.position;
//     
//     X_Vec3_fp16x16 newPos = x_vec3_add(&context->cam->base.position, &context->cam->base.velocity);
//     if(attempt_move_cam(context->engineContext, context->cam, newPos))
//     {
//         if(hitVerticalWall)
//         {
//             X_Vec3_fp16x16 adjustVelocity = context->cam->base.velocity;
//             X_Vec3_fp16x16 adjustPos = context->cam->base.position;
//             
//             const x_fp16x16 stepSize = x_fp16x16_from_int(18);
//             context->cam->base.position = oldPos;
//             context->cam->base.position.y -= stepSize;
//             context->cam->base.velocity = oldVelocity;
//             
//             newPos = x_vec3_add(&context->cam->base.position, &context->cam->base.velocity);
//             
//             attempt_move_cam(context->engineContext, context->cam, newPos);
//             
//             if(!onGround)
//             {
//                 context->cam->base.position = adjustPos;
//                 context->cam->base.velocity = adjustVelocity;
//                 x_cameraobject_update_view(context->cam);
//             }
//             
//             
//             //printf("Failed to move because of vertical wall\n");
//             
//         }
//     }
    
    //if(adjustCam)
    //{
    //    x_cameraobject_update_view(context->cam);
   // }
}

