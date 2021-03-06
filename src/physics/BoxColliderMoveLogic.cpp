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

#include "BoxColliderMoveLogic.hpp"
#include "level/BspLevel.hpp"
#include "level/Portal.hpp"

using BoxRayTracer = BspRayTracer;

static bool planeIsFloorSurface(const Plane& plane)
{
    const fp MAX_FLOOR_Y_NORMAL = fp::fromFloat(-0.7);
    return plane.normal.y <= MAX_FLOOR_Y_NORMAL;
}

static bool planeIsVertical(const Plane& plane)
{
    return plane.normal.y == 0;
}

bool BoxColliderMoveLogic::tryMoveNormally()
{
    newPosition = startingPosition + velocity * dt;

    runMoveIterations();

    if(!flags.hasFlag(IT_MOVE_SUCCESS))
    {
        comeToDeadStop();
    }
    
    tryStickToFloor();

    return flags.hasFlag(IT_MOVE_SUCCESS);
}

bool BoxColliderMoveLogic::tryMoveUpStep()
{
    // Try and move the collider up the height of the step and reattempt the move
    const fp MAX_STEP_SIZE = fp::fromFloat(18.0);

    startingPosition = startingPosition - Vec3fp(0, MAX_STEP_SIZE, 0);
    newPosition = startingPosition + velocity * dt;

    runMoveIterations();

    bool successfullyClimbedStep = flags.hasFlag(IT_MOVE_SUCCESS)
        && tryPushIntoGround(MAX_STEP_SIZE - fp::fromFloat(1))
        && planeIsFloorSurface(lastHitWall.plane);
    
    if(!successfullyClimbedStep)
    {
        return false;
    }
    
    // We successfully moved up the step, but we don't know how big the step was, so use the hit point
    // with the floor to reposition the collider
    newPosition = lastHitWall.location.point;

    return true;
}

void BoxColliderMoveLogic::moveAndAdjustVelocity()
{
    Ray3 movementRay = getMovementRay();

    if(!traceRay(movementRay, lastHitWall))
    {
        flags.set(IT_MOVE_SUCCESS);
        return;
    }

    if(planeIsVertical(lastHitWall.plane))
    {
        flags.set(IT_HIT_VERTICAL_WALL);
    }
    
    if(planeIsFloorSurface(lastHitWall.plane))
    {
        flags.set(IT_ON_FLOOR);
    }
    
    slideAlongWall();

    newPosition = lastHitWall.location.point + velocity * dt;
}

void BoxColliderMoveLogic::slideAlongWall()
{
    velocity = velocity
        + lastHitWall.plane.normal * (-lastHitWall.plane.normal.dot(velocity) * collider.bounceCoefficient);
}

Ray3 BoxColliderMoveLogic::getMovementRay()
{
    return Ray3(startingPosition, newPosition);
}

void BoxColliderMoveLogic::runMoveIterations()
{
    const int MAX_ITERATIONS = 4;

    for(int i = 0; i < MAX_ITERATIONS && !flags.hasFlag(IT_MOVE_SUCCESS); ++i)
    {
        moveAndAdjustVelocity();
    }
}

bool BoxColliderMoveLogic::onFloor()
{
    const fp MAX_FLOOR_DISTANCE_EPSILON = fp::fromFloat(1.0);

    return velocity.y >= 0
        && tryPushIntoGround(MAX_FLOOR_DISTANCE_EPSILON)
        && planeIsFloorSurface(lastHitWall.plane);
}

void BoxColliderMoveLogic::comeToDeadStop()
{
    velocity = Vec3fp(0, 0, 0);
}

void BoxColliderMoveLogic::tryStickToFloor()
{
    if(!onFloor())
    {
        return;
    }
    
    slideAlongWall();
    newPosition = lastHitWall.location.point;
    
    flags.set(IT_ON_FLOOR);
}

bool BoxColliderMoveLogic::tryPushIntoGround(fp distance)
{
    Ray3 movementRay(
        newPosition,
        newPosition + Vec3fp(0, distance, 0));

    bool isOnGround = traceRay(movementRay, lastHitWall);

    standingOnEntity = isOnGround
        ? lastHitWall.entity
        : nullptr;

    return isOnGround;
}

bool BoxColliderMoveLogic::traceRay(const Ray3& ray, RayCollision& collision)
{
    BoxRayTracer tracer(ray, &level, 0);

    bool hitSomething = tracer.trace();
    auto& tracerCollision = tracer.getCollision();

    if(tracerCollision.triggerCollision.hitTrigger)
    {
        triggerCollision = tracerCollision.triggerCollision;
    }

    if(hitSomething)
    {
        collision = tracerCollision;
    }

    return hitSomething;
}

Portal* BoxColliderMoveLogic::findWhichPortalBoxInside()
{
    for(Portal* portal = level.portalHead; portal != nullptr; portal = portal->next)
    {
        if(portal->pointInBox(newPosition))
        {
            return portal;
        }
    }

    return nullptr;
}

