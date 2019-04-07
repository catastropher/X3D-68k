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

#include "engine/EngineContext.hpp"
#include "physics/PhysicsEngine.hpp"
#include "entity/EntityBuilder.hpp"
#include "Player.hpp"

// FIXME: move into physics engine
bool physics = true;

fp moveSpeed;
fp gravity;
fp friction;
fp jump;
fp maxSpeed;

bool playerPhysics = true;

Entity* Player::build(EntityBuilder& builder)
{
    Player* player = builder
        .withComponent<TransformComponent>()
        .withInputComponent(handleKeys)
        .withComponent<BoxColliderComponent>()
        .withComponent<CameraComponent>()
        .withComponent<ScriptableComponent>()
        .build<Player>();

    CameraComponent* cam = player->getComponent<CameraComponent>();

    cam->viewport.init(Vec2(0, 0), 640, 480, X_ANG_60);

    player->angleX = 0;
    player->angleY = 0;

    player->getComponent<ScriptableComponent>()->handleEvent = handleEvent;

#if false
    auto collider = addComponent<BoxColliderComponent>();
    auto input = addComponent<InputComponent>(handleKeys);


    Vec3fp position;
    if(!edict.getValueOrDefault("origin", position, Vec3fp(0, 0, 0)))
    {
        x_system_error("No player start");
    }

    position.y -= fp::fromInt(30);

    getTransform().setPosition(position);

    CameraComponent* cam = addComponent<CameraComponent>();


    //, {16,16,24} },
    Vec3i mins(-16, -32, -16);
    Vec3i maxs(16, 24, 16);

    auto transform = getTransform();
    transform.setBoundBox(BoundBoxTemplate<fp>(mins.toVec3<fp>(), maxs.toVec3<fp>()));

    flags.set(EntityFlags::canPickThingsUp);
#endif

    return player;
}

void Player::registerVars()
{
    EngineContext* engineContext = Engine::getInstance();
    Console* console = engineContext->console;

    x_console_register_var(console, &moveSpeed, "player.speed", X_CONSOLEVAR_FP, "200", false);
    x_console_register_var(console, &gravity, "player.gravity", X_CONSOLEVAR_FP, "320", false);
    x_console_register_var(console, &friction, "player.friction", X_CONSOLEVAR_FP, "20", false);
    x_console_register_var(console, &jump, "player.jump", X_CONSOLEVAR_FP, "160", false);
    x_console_register_var(console, &maxSpeed, "player.maxspeed", X_CONSOLEVAR_FP, "300", false);

    x_console_register_var(console, &playerPhysics, "player.physics", X_CONSOLEVAR_BOOL, "1", true);
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

void mouseLook(Player* player, Vec2fp angleOffset)
{
    player->angleX += angleOffset.x;
    player->angleY += angleOffset.y;

    adjustAngle(player->angleX);
    adjustAngle(player->angleY);
}

void handle_mouse(Player* player, MouseState* mouseState)
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
    PlayerMoveLogic moveLogic(*this, moveSpeed, playerPhysics, update.deltaTime);
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

    handle_mouse(player, Engine::getInstance()->mouseState);
    player->handleMovement(update);

    return true;
}

EntityEventResponse Player::handleEvent(Entity& entity, const EntityEvent& event)
{
    switch(event.type)
    {
        case TriggerEntityEvent::Name:
            Engine::getInstance()->messageQueue->addMessage("Player hit trigger %d", event.to<const TriggerEntityEvent>()->hitEntity->getId());

            return EntityEventResponse::accepted;

        default:
            return EntityEventResponse::unhandled;
    }
}

