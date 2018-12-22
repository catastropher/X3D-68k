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

#include "X_BoxColliderEngine.hpp"

void BoxColliderEngine::runStep()
{
    resetCollisionState();
    applyGravity();
    tryMove();
}

void BoxColliderEngine::tryMove()
{
    BoxColliderMoveLogic moveLogic(
        collider,
        level,
        collider.position,
        collider.velocity);

    moveLogic.tryMoveNormally();

    if(moveLogic.potentiallyHitStep())
    {
        BoxColliderMoveLogic stepMoveLogic(
            collider,
            level,
            collider.position,
            collider.velocity);

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
    collider.position = moveLogic.getFinalPosition();
    collider.velocity = moveLogic.getFinalVelocity();
    
    auto moveFlags = moveLogic.getMovementFlags();

    if(moveFlags.hasFlag(IT_ON_FLOOR))
    {
        applyFriction();
        collider.flags.set(X_BOXCOLLIDER_ON_GROUND);

        auto lastHitWall = moveLogic.getLastHitWall();

        linkToModelStandingOn(lastHitWall.hitModel);
    }
    else
    {
        collider.flags.reset(X_BOXCOLLIDER_ON_GROUND);
    }
}

void BoxColliderEngine::applyFriction()
{
    fp currentSpeed = collider.velocity.length();
    if(currentSpeed == 0)
    {
        return;
    }
    
    fp newSpeed = currentSpeed - collider.frictionCoefficient;
    newSpeed = clamp(newSpeed, fp::fromInt(0), collider.maxSpeed);
    
    newSpeed = newSpeed / currentSpeed;

    collider.velocity = collider.velocity * newSpeed;
}

void x_boxcollider_init(X_BoxCollider* collider, BoundBox* boundBox, EnumBitSet<X_BoxColliderFlags> flags)
{
    static Vec3fp gravity = { 0, fp::fromFloat(0.25), 0 };
    
    collider->boundBox = *boundBox;
    collider->flags = flags;
    collider->gravity = &gravity;
    collider->frictionCoefficient = x_fp16x16_from_float(4.0);
    collider->maxSpeed = x_fp16x16_from_float(20.0);
    collider->bounceCoefficient = X_FP16x16_ONE;
    collider->collisionInfo.type = BOXCOLLIDER_COLLISION_NONE;
    
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

void BoxColliderEngine::linkToModelStandingOn(X_BspModel* model)
{
    x_link_insert_after(&collider.objectsOnModel, &model->objectsOnModelHead);
}

void BoxColliderEngine::applyGravity()
{
    collider.velocity += *collider.gravity;
}

void x_boxcollider_update(X_BoxCollider* collider, X_BspLevel* level)
{
    BoxColliderEngine engine(*collider, *level);
    engine.runStep();
}


