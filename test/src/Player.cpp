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
bool physics = true;

fp moveSpeed;
fp gravity;
fp friction;
fp jump;
fp maxSpeed;

void Player::registerVars()
{
    X_EngineContext* engineContext = Engine::getInstance();
    Console* console = engineContext->getConsole();

    x_console_register_var(console, &moveSpeed, "player.speed", X_CONSOLEVAR_FP16X16, "200", false);
    x_console_register_var(console, &gravity, "player.gravity", X_CONSOLEVAR_FP16X16, "320", false);
    x_console_register_var(console, &friction, "player.friction", X_CONSOLEVAR_FP16X16, "50", false);
    x_console_register_var(console, &jump, "player.jump", X_CONSOLEVAR_FP16X16, "160", false);
    x_console_register_var(console, &maxSpeed, "player.maxspeed", X_CONSOLEVAR_FP16X16, "300", false);
}

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

void mouseLook(Player* player, Vec2_fp16x16 angleOffset)
{
    player->angleX += fp(angleOffset.x);
    player->angleY += fp(angleOffset.y);

    fp x(player->angleX);
    adjustAngle(x);

    fp y(player->angleY);
    adjustAngle(y);

    player->angleX = x.toFp16x16();
    player->angleY = y.toFp16x16();
}

void handle_mouse(Player* player, X_MouseState* mouseState)
{
    mouseLook(
        player,
        x_mousestate_get_mouselook_angle_change(mouseState));
}

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
    PlayerMoveLogic moveLogic(*this, moveSpeed, true, update.deltaTime);
    PlayerKeyFlags keys = getPlayerKeys(update.keyState);

    auto& collider = getCollider();

    collider.maxSpeed = maxSpeed;
    collider.frictionCoefficient = friction;
    collider.gravity->y = gravity;

    StatusBar::setItem("Velocity", "%f %f %f", collider.velocity.x.toFloat(), collider.velocity.y.toFloat(), collider.velocity.z.toFloat());
    StatusBar::setItem("Friction", "%f", collider.frictionCoefficient.toFloat());

    moveLogic.applyMovement(keys, &getLevel());

    Quaternion newOrientation = Quaternion::fromEulerAngles(angleX, angleY, 0);

    getTransform().setOrientation(newOrientation);
}

bool Player::handleKeys(Entity* entity, const InputUpdate& update)
{
    Player* player = static_cast<Player*>(entity);

    handle_mouse(player, Engine::getInstance()->getMouseState());
    player->handleMovement(update);

    return true;
}


