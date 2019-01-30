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

#include "Player.hpp"

// FIXME: move into physics engine
bool physics = true ;

PlayerKeyFlags getPlayerKeys(KeyState* keyState)
{
    PlayerKeyFlags keys = 0;

    if(x_keystate_key_down(keyState, KeyCode::left))
    {
        keys.set(PlayerKeys::lookLeft);
    }
    else if(x_keystate_key_down(keyState, KeyCode::right))
    {
        keys.set(PlayerKeys::lookRight);
    }

    if(x_keystate_key_down(keyState, KeyCode::up))
    {
        keys.set(PlayerKeys::lookUp);
    }
    else if(x_keystate_key_down(keyState, KeyCode::down))
    {
        keys.set(PlayerKeys::lookDown);
    }

    if(x_keystate_key_down(keyState, (KeyCode)' '))
    {
        keys.set(PlayerKeys::jump);
    }

    if(x_keystate_key_down(keyState, (KeyCode)'w'))
    {
        keys.set(PlayerKeys::forward);
    }
    else if(x_keystate_key_down(keyState, (KeyCode)'s'))
    {
        keys.set(PlayerKeys::backward);
    }

    if(x_keystate_key_down(keyState, (KeyCode)'a'))
    {
        keys.set(PlayerKeys::strafeLeft);
    }
    else if(x_keystate_key_down(keyState, (KeyCode)'d'))
    {
        keys.set(PlayerKeys::strafeRight);
    }

    return keys;
}

//void mouseLook(Player* player, Vec2_fp16x16 angleOffset, fp timeDelta)
//{
//    player->angleX += fp(angleOffset.x);
//    player->angleY += fp(angleOffset.y);
//
//    fp x(player->angleX);
//    adjustAngle(x);
//
//    fp y(player->angleY);
//    adjustAngle(y);
//
//    player->angleX = x.toFp16x16();
//    player->angleY = y.toFp16x16();
//}

//void handle_mouse(Context* context)
//{
//    X_MouseState* state = context->engineContext->getMouseState();
//    mouseLook(
//        context->player,
//        x_mousestate_get_mouselook_angle_change(state),
//        context->engineContext->timeDelta);
//}

#include "Player.hpp"
#include "PlayerMoveLogic.hpp"

void handle_keys()
{
//    x_platform_handle_keys(context->engineContext);
//    x_platform_handle_mouse(context->engineContext);
//
//    if(x_keystate_key_down(context->engineContext->getKeyState(), escape))
//        context->quit = 1;
//
//    if(handle_console(context->engineContext))
//        return;
//
//    KeyState* keyState = context->engineContext->getKeyState();
//
//
//    handle_mouse(context);
//
//    PlayerMoveLogic moveLogic(*context->player, context->moveSpeed, physics, context->engineContext->timeDelta);
//
//    PlayerKeyFlags keys = getPlayerKeys(keyState);
//
//    moveLogic.applyMovement(keys, x_engine_get_current_level(context->engineContext));
}

void Player::handleMovement(const InputUpdate& update)
{
    PlayerMoveLogic moveLogic(*this, fp::fromFloat(100), true, update.deltaTime);
    PlayerKeyFlags keys = getPlayerKeys(update.keyState);

    moveLogic.applyMovement(keys, &getLevel());

    Quaternion newOrientation = Quaternion::fromEulerAngles(angleX, angleY, 0);

    getTransform().setOrientation(newOrientation);
}

bool Player::handleKeys(Entity* entity, const InputUpdate& update)
{
    Player* player = static_cast<Player*>(entity);

    player->handleMovement(update);

    return true;
}


