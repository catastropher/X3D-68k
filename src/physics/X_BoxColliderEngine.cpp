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

static bool planeIsFloorSurface(const Plane& plane)
{
    const fp MAX_FLOOR_Y_NORMAL = fp::fromFloat(-0.7);
    return plane.normal.y <= MAX_FLOOR_Y_NORMAL;
}

bool BoxColliderEngine::tryMove()
{
    BoxColliderState state(collider.position, collider.velocity);
    
    runMoveIterations(state);

    if(state.flags.isSet(IT_HIT_VERTICAL_WALL))
    {
        if(tryMoveUpStep(state))
        {
            state.flags.set(IT_MOVE_SUCCESS);
        }
    }

    if(!state.flags.isSet(IT_MOVE_SUCCESS))
    {
        comeToDeadStop(state);
    }
    
    tryStickToFloor(state);
    
    if(state.flags.isSet(IT_ON_FLOOR))
    {
        applyFriction(state);
        collider.flags.set(X_BOXCOLLIDER_ON_GROUND);
    }
    else
    {
        collider.flags.reset(X_BOXCOLLIDER_ON_GROUND);
    }

    collider.position = state.newPosition;
    collider.velocity = state.velocity;
    
    return true;
}

bool BoxColliderEngine::tryMoveUpStep(BoxColliderState& state)
{
    const fp MAX_STEP_SIZE = fp::fromFloat(18.0);
    
    // Try and move the collider up the height of the step and reattempt the move
    Vec3fp newPosition = state.position - Vec3fp(0, MAX_STEP_SIZE, 0);

    BoxColliderState newState(newPosition, collider.velocity);

    runMoveIterations(newState);

    bool successfullyClimbedStep = newState.flags.isSet(IT_MOVE_SUCCESS)
        && tryPushIntoGround(newState.newPosition, MAX_STEP_SIZE - fp::fromFloat(1))
        && planeIsFloorSurface(lastHitWall.plane);
    
    if(!successfullyClimbedStep)
    {
        return false;
    }
    
    // We successfully moved up the step, but we don't know how big the step was, so use the hit point
    // with the floor to reposition the collider
    state.newPosition = lastHitWall.location.point;
    state.velocity = newState.velocity;
    state.flags = newState.flags;

    return true;
}


void BoxColliderEngine::comeToDeadStop(BoxColliderState& state)
{
    state.velocity = Vec3fp(0, 0, 0);
}

void BoxColliderEngine::tryStickToFloor(BoxColliderState& state)
{
    if(!onFloor(state))
    {
        return;
    }
    
    state.newPosition = lastHitWall.location.point;
    slideAlongWall(state);
    //link_to_model_standing_on(collider, floor.hitModel);
    
    state.flags.set(IT_ON_FLOOR);
}

bool BoxColliderEngine::onFloor(BoxColliderState& state)
{
    const fp MAX_FLOOR_DISTANCE_EPSILON = fp::fromFloat(3.0);

    return tryPushIntoGround(state.newPosition, MAX_FLOOR_DISTANCE_EPSILON)
        && planeIsFloorSurface(lastHitWall.plane);
}

void BoxColliderEngine::applyFriction(BoxColliderState& state)
{
    fp currentSpeed = state.velocity.length();
    if(currentSpeed == 0)
    {
        return;
    }
    
    fp newSpeed = currentSpeed - collider.frictionCoefficient;
    newSpeed = clamp(newSpeed, fp::fromInt(0), collider.maxSpeed);
    
    newSpeed = newSpeed / currentSpeed;

    state.velocity = state.velocity * newSpeed;
}

bool BoxColliderEngine::tryPushIntoGround(Vec3fp& startLocation, fp distance)
{
    Ray3 movementRay(
        startLocation,
        startLocation + Vec3fp(0, distance, 0));

    return traceRay(movementRay, lastHitWall);
}

static bool planeIsVertical(const Plane& plane)
{
    return plane.normal.y == 0;
}

bool BoxColliderEngine::traceRay(const Ray3& ray, RayCollision<int>& collision)
{
    BoxRayTracer tracer(ray, &level, 0);

    bool hitSomething = tracer.trace();

    if(hitSomething)
    {
        collision = tracer.getCollision();
    }

    return hitSomething;
}

void BoxColliderEngine::moveAndAdjustVelocity(BoxColliderState& state)
{
    Ray3 movementRay = getMovementRay(state);

    if(!traceRay(movementRay, lastHitWall))
    {
        state.flags.set(IT_MOVE_SUCCESS);
        return;
    }

    if(planeIsVertical(lastHitWall.plane))
    {
        state.flags.set(IT_HIT_VERTICAL_WALL);
    }
    
    if(planeIsFloorSurface(lastHitWall.plane))
    {
        state.flags.set(IT_ON_FLOOR);
    }
    
    slideAlongWall(state);

    state.newPosition = lastHitWall.location.point + state.velocity;
}

void BoxColliderEngine::slideAlongWall(BoxColliderState& state)
{
    state.velocity = state.velocity
        + lastHitWall.plane.normal * (-lastHitWall.plane.normal.dot(state.velocity) * collider.bounceCoefficient);
}

Ray3 BoxColliderEngine::getMovementRay(BoxColliderState& state)
{
    return Ray3(state.position, state.newPosition);
}

void BoxColliderEngine::runMoveIterations(BoxColliderState& state)
{
    const int MAX_ITERATIONS = 4;

    for(int i = 0; i < MAX_ITERATIONS && !state.flags.isSet(IT_MOVE_SUCCESS); ++i)
    {
        moveAndAdjustVelocity(state);
    }
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

void BoxColliderEngine::linkToModelStandingOn()
{
    x_link_insert_after(&collider.objectsOnModel, &lastHitWall.hitModel->objectsOnModelHead);
}

void BoxColliderEngine::step()
{
    resetCollisionState();
    applyGravity();
    tryMove();
}

void BoxColliderEngine::applyGravity()
{
    collider.velocity += *collider.gravity;
}

void x_boxcollider_update(X_BoxCollider* collider, X_BspLevel* level)
{
    BoxColliderEngine engine(*collider, *level);
    engine.step();
}


