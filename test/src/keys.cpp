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
    
    x_console_register_var(context->engineContext->getConsole(), &g_Context->moveSpeed, "moveSpeed", X_CONSOLEVAR_FP16X16, "3.0", 0);
    x_console_register_var(context->engineContext->getConsole(), &physics, "physics", X_CONSOLEVAR_BOOL, "1", 0);
}

void cleanup_keys(Context* context)
{
    
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

bool handle_console(X_EngineContext* engineContext)
{
    if(engineContext->getConsole()->isOpen())
    {
        handle_console_keys(engineContext);
        return 1;
    }
    
    if(x_keystate_key_down(engineContext->getKeyState(), X_KEY_OPEN_CONSOLE))
    {
        x_console_open(engineContext->getConsole());
        x_keystate_reset_keys(engineContext->getKeyState());
        x_keystate_enable_text_input(engineContext->getKeyState());
        return 1;
    }
    
    return 0;
}

void mouseLook(Player* player, X_Vec2_fp16x16 angleOffset)
{
    player->angleX += angleOffset.x;
    player->angleY += angleOffset.y;
    
    fp x(player->angleX);
    adjustAngle(x);
    
    fp y(player->angleY);
    adjustAngle(y);
    
    player->angleX = x.toFp16x16();
    player->angleY = y.toFp16x16();
}

void handle_mouse(Context* context)
{
    X_MouseState* state = context->engineContext->getMouseState();
    mouseLook(context->player, x_mousestate_get_mouselook_angle_change(state));
}

#include "Player.hpp"
#include "PlayerMoveLogic.hpp"

PlayerKeyFlags getPlayerKeys(X_KeyState* keyState)
{
    PlayerKeyFlags keys = 0;
    
    if(x_keystate_key_down(keyState, X_KEY_LEFT))
    {
        keys.set(PlayerKeys::lookLeft);
    }
    else if(x_keystate_key_down(keyState, X_KEY_RIGHT))
    {
        keys.set(PlayerKeys::lookRight);
    }
    
    if(x_keystate_key_down(keyState, X_KEY_UP))
    {
        keys.set(PlayerKeys::lookUp);
    }
    else if(x_keystate_key_down(keyState, X_KEY_DOWN))
    {
        keys.set(PlayerKeys::lookDown);
    }
    
    if(x_keystate_key_down(keyState, (X_Key)' '))
    {
        keys.set(PlayerKeys::jump);
    }
    
    if(x_keystate_key_down(keyState, (X_Key)'w'))
    {
        keys.set(PlayerKeys::forward);
    }
    else if(x_keystate_key_down(keyState, (X_Key)'s'))
    {
        keys.set(PlayerKeys::backward);
    }
    
    if(x_keystate_key_down(keyState, (X_Key)'a'))
    {
        keys.set(PlayerKeys::strafeLeft);
    }
    else if(x_keystate_key_down(keyState, (X_Key)'d'))
    {
        keys.set(PlayerKeys::strafeRight);
    }
    
    return keys;
}

void handle_keys(Context* context)
{
    x_platform_handle_keys(context->engineContext);
    x_platform_handle_mouse(context->engineContext);
    
    if(x_keystate_key_down(context->engineContext->getKeyState(), X_KEY_ESCAPE))
        context->quit = 1;
    
    if(handle_console(context->engineContext))
        return;
    
    X_KeyState* keyState = context->engineContext->getKeyState();

    
    handle_mouse(context);
    
    PlayerMoveLogic moveLogic(*context->player, context->moveSpeed);
    
    PlayerKeyFlags keys = getPlayerKeys(keyState);
    
    moveLogic.applyMovement(keys, x_engine_get_current_level(context->engineContext));
    
    
//     if(!x_demoplayer_is_playing(&g_Context->demoPlayer))
//       handle_angle_keys(context->cam, keyState);
//     
//     if(handle_no_collision_keys(context->engineContext, context->cam, keyState))
//         return;
//     
//     handle_normal_movement(context->engineContext, context->cam);
}

