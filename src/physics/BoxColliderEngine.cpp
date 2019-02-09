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

#include "BoxColliderEngine.hpp"
#include "entity/Entity.hpp"
#include "render/StatusBar.hpp"

void BoxColliderEngine::runStep()
{
    if(collider.flags.hasFlag(X_BOXCOLLIDER_ON_GROUND) && collider.velocity.y >= fp::fromInt(0))
    {
        applyFriction();
    }

    fp currentSpeed = collider.velocity.length();

    if(currentSpeed != 0)
    {
        fp clampedSpeed = clamp(currentSpeed, fp(0), collider.maxSpeed);
        fp t = clampedSpeed / currentSpeed;

        collider.velocity = collider.velocity * t;

        currentSpeed = clampedSpeed;
    }

    StatusBar::setItem("Current speed", "%f", currentSpeed.toFloat());

    if(collider.transformComponentId == COMPONENT_INVALID_ID)
    {
        collider.transformComponentId = collider.owner->getComponentId<TransformComponent>();
    }
    
    transformComponent = TransformComponent::getById(collider.transformComponentId);
    
    if(transformComponent == nullptr)
    {
        return;
    }
    
    resetCollisionState();
    applyGravity();
    tryMove();
}

void BoxColliderEngine::tryMove()
{
    BoxColliderMoveLogic moveLogic(
        collider,
        level,
        transformComponent->getPosition(),
        collider.velocity,
        dt);

    moveLogic.tryMoveNormally();

    if(moveLogic.potentiallyHitStep())
    {
        BoxColliderMoveLogic stepMoveLogic(
            collider,
            level,
            transformComponent->getPosition(),
            collider.velocity,
            dt);

        if(stepMoveLogic.tryMoveUpStep())
        {
            useResultsFromMoveLogic(stepMoveLogic);
            return;
        }
    }

    useResultsFromMoveLogic(moveLogic);
}

void BoxColliderEngine::useResultsFromMoveLogic(BoxColliderMoveLogic& moveLogic)
{
    transformComponent->setPosition(moveLogic.getFinalPosition());
    collider.velocity = moveLogic.getFinalVelocity();
    
    auto moveFlags = moveLogic.getMovementFlags();

    if(moveFlags.hasFlag(IT_ON_FLOOR))
    {
        collider.flags.set(X_BOXCOLLIDER_ON_GROUND);

        auto lastHitWall = moveLogic.getLastHitWall();

        linkToModelStandingOn(lastHitWall.hitModel);

        collider.standingOnEntity = lastHitWall.entity;
    }
    else
    {
        collider.flags.reset(X_BOXCOLLIDER_ON_GROUND);

        collider.standingOnEntity = nullptr;
    }
}

void BoxColliderEngine::applyFriction()
{
    fp currentSpeed = collider.velocity.length();
    if(currentSpeed == 0)
    {
        return;
    }


    fp newSpeed = currentSpeed - currentSpeed * collider.frictionCoefficient * dt;
    newSpeed = clamp(newSpeed, fp::fromInt(0), collider.maxSpeed);

    newSpeed = newSpeed / currentSpeed;

    collider.velocity = collider.velocity * newSpeed;
}

void x_boxcollider_init(X_BoxCollider* collider, BoundBox* boundBox, EnumBitSet<X_BoxColliderFlags> flags)
{
    static Vec3fp gravity = { 0, fp::fromFloat(320), 0 };
    
    collider->boundBox = *boundBox;
    collider->flags = flags;
    collider->gravity = &gravity;
    collider->frictionCoefficient = x_fp16x16_from_float(50.0);
    collider->maxSpeed = x_fp16x16_from_float(20.0);
    collider->bounceCoefficient = X_FP16x16_ONE;
    collider->collisionInfo.type = BOXCOLLIDER_COLLISION_NONE;
    collider->velocity = Vec3fp(0, 0, 0);
    collider->impulseVelocity = Vec3fp(0, 0, 0);    // TODO: remove
    collider->frameVelocity = Vec3fp(0, 0, 0);
    
    x_link_init_self(&collider->objectsOnModel);
}

void BoxColliderEngine::resetCollisionState()
{
    collider.collisionInfo.type = BOXCOLLIDER_COLLISION_NONE;
    unlinkFromModelStandingOn();
}

void BoxColliderEngine::unlinkFromModelStandingOn()
{
    x_link_unlink(&collider.objectsOnModel);
}

void BoxColliderEngine::linkToModelStandingOn(BspModel* model)
{
    x_link_insert_after(&collider.objectsOnModel, &model->objectsOnModelHead);
}

void BoxColliderEngine::applyGravity()
{
    if(collider.flags.hasFlag(X_BOXCOLLIDER_APPLY_GRAVITY))
    {
        collider.velocity += *collider.gravity * dt;
    }
}

