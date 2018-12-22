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

#ifdef __nspire__

#define KEY_FORWARD '7'
#define KEY_BACKWARD '4'

#else

#define KEY_FORWARD 'w'
#define KEY_BACKWARD 's'

#endif

static Context* g_Context;

bool physics = 1;

void init_keys(Context* context)
{
    g_Context = context;
    
    x_demorecorder_init(&context->demoRecorder, context->cam, context->engineContext->getKeyState());
    x_demoplayer_init(&context->demoPlayer, context->cam, context->engineContext->getKeyState());
    
    x_console_register_var(context->engineContext->getConsole(), &g_Context->moveSpeed, "moveSpeed", X_CONSOLEVAR_FP16X16, "3.0", 0);
    x_console_register_var(context->engineContext->getConsole(), &physics, "physics", X_CONSOLEVAR_BOOL, "1", 0);
}

void cleanup_keys(Context* context)
{
    x_demorecorder_cleanup(&context->demoRecorder);
    x_demoplayer_cleanup(&context->demoPlayer);
}

void handle_console_keys(X_EngineContext* context)
{
    X_Key key;
    while(x_keystate_dequeue(context->getKeyState(), &key))
    {
        if(key == X_KEY_OPEN_CONSOLE)
        {
            x_console_close(context->getConsole());
            x_keystate_reset_keys(context->getKeyState());
            x_keystate_disable_text_input(context->getKeyState());
            return;
        }
        
        x_console_send_key(context->getConsole(), key);
    }
}

static void keep_horizontal_component(Vec3* v)
{
    v->y = 0;
    x_vec3_normalize(v);
}

static x_fp16x16 get_forward_component(X_KeyState* keyState)
{
    if(x_keystate_key_down(keyState, (X_Key)KEY_FORWARD))
        return X_FP16x16_ONE;
    else if(x_keystate_key_down(keyState, (X_Key)KEY_BACKWARD))
        return -X_FP16x16_ONE;
    
    return 0;
}

static x_fp16x16 get_straife_component(X_KeyState* keyState)
{
    if(x_keystate_key_down(keyState, (X_Key)'d'))
        return X_FP16x16_ONE;
    else if(x_keystate_key_down(keyState, (X_Key)'a'))
        return -X_FP16x16_ONE;
    
    return 0;
}

static Vec3 get_movement_key_vector(X_CameraObject* cam, X_KeyState* keyState, bool ignoreVerticalComponent)
{
    x_fp16x16 forwardComponent = get_forward_component(keyState);
    x_fp16x16 rightComponent = get_straife_component(keyState);
    
    Vec3 forward, right, up;

    Vec3fp f, r, u;

    cam->viewMatrix.extractViewVectors(f, r, u);

    forward = MakeVec3(f);
    right = MakeVec3(r);
    up = MakeVec3(u);
    
    if(ignoreVerticalComponent)
    {
        keep_horizontal_component(&forward);
        keep_horizontal_component(&right);
    }
    
    Vec3 totalVelocity = x_vec3_origin();
    totalVelocity = x_vec3_add_scaled(&totalVelocity, &forward, forwardComponent);
    totalVelocity = x_vec3_add_scaled(&totalVelocity, &right, rightComponent);
    
    if(totalVelocity.x == 0 && totalVelocity.y == 0 && totalVelocity.z == 0)
        return totalVelocity;
    
    x_vec3_normalize(&totalVelocity);
    totalVelocity = x_vec3_fp16x16_scale(&totalVelocity, g_Context->moveSpeed);
    
    return totalVelocity;
}

static Vec3 get_jump_vector(X_KeyState* keyState)
{
    if(!x_keystate_key_down(keyState, (X_Key)' '))
        return x_vec3_origin();
    
    x_fp16x16 jumpVelocity = -x_fp16x16_from_float(6.0);
    return Vec3(0, jumpVelocity, 0);
}

static Vec3 get_movement_vector(X_EngineContext* context, X_CameraObject* cam)
{
    if(!x_boxcollider_is_on_ground(&cam->collider))
        return x_vec3_origin();
    
    X_KeyState* keyState = context->getKeyState();
    
    Vec3 moveVelocity = get_movement_key_vector(cam, keyState, 1);
    Vec3 jumpVelocity = get_jump_vector(keyState);
    
    return moveVelocity + jumpVelocity;
}

void handle_demo(Context* context)
{
    bool wasPlaying = 0;
    
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
        x_console_execute_cmd(context->engineContext->getConsole(), "endrecord");
    }
}

void handle_angle_keys(X_CameraObject* cam, X_KeyState* keyState)
{    
    x_fp16x16 dAngle = x_fp16x16_from_float(2.0);
    
    if(x_keystate_key_down(keyState, X_KEY_UP))
        cam->angleX -= dAngle;
    else if(x_keystate_key_down(keyState, X_KEY_DOWN))
        cam->angleX += dAngle;
    
    if(x_keystate_key_down(keyState, X_KEY_LEFT))
        cam->angleY += dAngle;
    else if(x_keystate_key_down(keyState, X_KEY_RIGHT))
        cam->angleY -= dAngle;
}

bool handle_console(X_EngineContext* engineContext)
{
    // if(x_console_is_open(engineContext->getConsole()))
    // {
    //     handle_console_keys(engineContext);
    //     return 1;
    // }
    
    if(x_keystate_key_down(engineContext->getKeyState(), X_KEY_OPEN_CONSOLE))
    {
        x_console_open(engineContext->getConsole());
        x_keystate_reset_keys(engineContext->getKeyState());
        x_keystate_enable_text_input(engineContext->getKeyState());
        return 1;
    }
    
    return 0;
}

bool handle_no_collision_keys(X_EngineContext* engineContext, X_CameraObject* cam, X_KeyState* keyState)
{
    if(x_engine_level_is_loaded(engineContext) && physics)
    {
        Vec3fp camPos = MakeVec3fp(x_cameraobject_get_position(cam));
        
        if(engineContext->getCurrentLevel()->findLeafPointIsIn(camPos)->contents != X_BSPLEAF_SOLID)
            return 0;
    }
    
    Vec3 movementVector = get_movement_key_vector(cam, keyState, 0);
    cam->collider.position += MakeVec3fp(movementVector);
    cam->updateView();
    
    return 1;
}

void handle_normal_movement(X_EngineContext* engineContext, X_CameraObject* cam)
{
    Vec3 movementVector = get_movement_vector(engineContext, cam);
    cam->collider.velocity = cam->collider.velocity + MakeVec3fp(movementVector);
    x_boxcollider_update(&cam->collider, engineContext->getCurrentLevel());

    if(cam->collider.collisionInfo.type == BOXCOLLIDER_COLLISION_PORTAL)
    {
        auto portal = cam->collider.collisionInfo.hitPortal;

        cam->angleX += portal->transformAngleX.toFp16x16();
        cam->angleY += portal->transformAngleY.toFp16x16();

        printf("Cam hit portal!\n");
    }
    
    cam->updateView();
}

void handle_mouse(Context* context)
{
    X_MouseState* state = context->engineContext->getMouseState();
    x_cameraobject_add_angle(context->cam, x_mousestate_get_mouselook_angle_change(state));
}

void handle_keys(Context* context)
{
    x_platform_handle_keys(context->engineContext);
    x_platform_handle_mouse(context->engineContext);
    
    handle_demo(context);
    
    if(x_keystate_key_down(context->engineContext->getKeyState(), X_KEY_ESCAPE))
        context->quit = 1;
    
    if(handle_console(context->engineContext))
        return;
    
    X_KeyState* keyState = context->engineContext->getKeyState();

    
    handle_mouse(context);
    
    if(!x_demoplayer_is_playing(&g_Context->demoPlayer))
      handle_angle_keys(context->cam, keyState);
    
    if(handle_no_collision_keys(context->engineContext, context->cam, keyState))
        return;
    
    handle_normal_movement(context->engineContext, context->cam);
}

